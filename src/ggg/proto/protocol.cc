#include <array>
#include <cstring>
#include <iostream>
#include <memory>
#include <thread>

#include <unistdx/io/poller>
#include <unistdx/net/socket>

#include <ggg/config.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/proto/kernel.hh>
#include <ggg/proto/protocol.hh>
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
    Frame frame;
    while (in.remaining() >= sizeof(Frame)) {
        std::memcpy(&frame, in.data()+in.position(), sizeof(Frame));
        // stop on partial frame
        if (in.remaining() < frame.size) { break; }
        // skip frames with invalid kernel types
        if (frame.command == Command::Unspecified || frame.command >= Command::Size) {
            in.bump(frame.size); continue;
        }
        auto constructor = all_constructors[size_t(frame.command)];
        auto kernel = constructor();
        auto old_limit = in.limit();
        in.limit(in.position() + frame.size);
        in.bump(sizeof(Frame));
        try {
            kernel->read(in);
            kernel->client_credentials(sock.credentials());
            kernel->run();
        } catch (const std::exception& err) {
            kernel->result(-1);
            log("kernel read/run error: _", err.what());
        }
        // move to the next frame
        in.position(in.limit());
        in.limit(old_limit);
        auto out_old_position = out.position();
        out.bump(sizeof(Frame));
        try {
            kernel->write(out);
            auto new_position = out.position();
            frame.size = new_position - out_old_position;
            out.position(out_old_position);
            out.write(&frame, sizeof(Frame));
            out.position(new_position);
        } catch (const std::exception& err) {
            out.position(out_old_position);
            log("kernel write error: _", err.what());
        }
    }
}

void
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
    }
    No_lock lock;
    auto deadline = system_clock::now() + seconds(7);
    sys::event_poller poller;
    poller.emplace(s.fd(), sys::event::inout);
    std::cv_status status;
    enum { Writing, Reading, Finish } state = Writing;
    buf.flush(s);
    if (buf.remaining() == 0) { buf.clear(); state = Reading; }
    while (state != Finish) {
        status = poller.wait_until(lock, deadline);
        if (status == std::cv_status::timeout) { kernel->result(-1); break; }
        for (const auto& event : poller) {
            if (state == Writing) {
                if (event.out()) {
                    buf.flush(s);
                    if (buf.remaining() == 0) {
                        buf.clear(); state = Reading;
                    }
                }
            } else {
                if (event.in()) {
                    buf.fill(s);
                    if (buf.position() < sizeof(Frame)) { continue; }
                    auto frame = reinterpret_cast<Frame*>(buf.data());
                    if (buf.position() < frame->size) { continue; }
                    buf.flip();
                    buf.read(frame, sizeof(Frame));
                    kernel->read(buf);
                    state = Finish;
                    break;
                }
            }
        }
    }
    s.shutdown(sys::shutdown_flag::read_write);
    s.close();
}
