#include <array>
#include <iostream>
#include <memory>
#include <thread>

#include <unistdx/io/poller>
#include <unistdx/net/socket>

#include <ggg/config.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/proto/kernel.hh>
#include <ggg/proto/protocol.hh>
#include <ggg/proto/result.hh>
#include <ggg/proto/selection.hh>

namespace {

    using kernel_ptr = std::unique_ptr<ggg::Kernel>;
    using constructor = kernel_ptr(*)();

    std::array<constructor,size_t(ggg::Protocol::Command::Size)> all_constructors{
        nullptr,
        [] () { return kernel_ptr(new ggg::PAM_kernel); },
        [] () { return kernel_ptr(new ggg::NSS_kernel); },
    };

    struct No_lock { void lock() {} void unlock() {} };

}

void
ggg::Server_protocol::process(sys::socket& sock, sys::byte_buffer& in, sys::byte_buffer& out) {
    if (in.remaining() < sizeof(sys::u32)) { return; }
    auto frame = reinterpret_cast<Frame*>(in.data());
    if (in.remaining() < frame->size) { return; }
    if (frame->command == Command::Result) { return; }
    if (frame->command >= Command::Size) { return; }
    auto constructor = all_constructors[size_t(frame->command)];
    if (!constructor) { return; }
    auto kernel = constructor();
    in.limit(in.position() + frame->size);
    in.position(sizeof(Frame));
    try {
        kernel->read(in);
        kernel->client_credentials(sock.credentials());
        kernel->run();
    } catch (const std::exception& err) {
        kernel->result(1);
        log("kernel error: _", err.what());
    }
    auto old_position = out.position();
    out.bump(sizeof(Frame));
    kernel->write(out);
    auto new_position = out.position();
    frame->size = new_position - old_position;
    out.position(old_position);
    out.write(frame, sizeof(Frame));
    out.position(new_position);
}

sys::u32
ggg::Client_protocol::process(Kernel* kernel, Command command) {
    using namespace std::chrono;
    sys::socket_address address(GGG_BIND_ADDRESS);
    sys::socket s(sys::family_type::unix);
    s.setopt(sys::socket::pass_credentials);
    s.connect(address);
    sys::byte_buffer buf{4096};
    Frame frame;
    {
        frame.command = command;
        auto old_position = buf.position();
        buf.bump(sizeof(Frame));
        kernel->write(buf);
        auto new_position = buf.position();
        frame.size = new_position - old_position;
        buf.position(old_position);
        buf.write(&frame, sizeof(Frame));
        buf.position(new_position);
        buf.flip();
        buf.flush(s);
    }
    No_lock lock;
    auto deadline = system_clock::now() + seconds(30);
    sys::event_poller poller;
    poller.emplace(s.fd(), sys::event::inout);
    std::cv_status status;
    enum { Writing, Reading, Finish } state = Writing;
    if (buf.remaining() == 0) { buf.clear(); state = Reading; }
    while (state != Finish) {
        status = poller.wait_until(lock, deadline);
        if (status == std::cv_status::timeout) { kernel->result(1); break; }
        for (const auto& event : poller) {
            if (state == Writing) {
                if (event.out()) {
                    buf.flush(s);
                    if (buf.remaining() == 0) { buf.clear(); state = Reading; }
                }
            } else {
                if (event.in()) {
                    buf.fill(s);
                    if (buf.position() < sizeof(sys::u32)) { continue; }
                    auto frame = reinterpret_cast<Frame*>(buf.data());
                    if (buf.position() < frame->size) { continue; }
                    buf.flip();
                    buf.read(frame, sizeof(Frame));
                    kernel->read(buf);
                    state = Finish;
                }
            }
        }
    }
    return kernel->result();
}
