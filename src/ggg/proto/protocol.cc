#include <array>
#include <iostream>
#include <memory>
#include <thread>

#include <unistdx/net/socket>

#include <ggg/config.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/proto/kernel.hh>
#include <ggg/proto/protocol.hh>
#include <ggg/proto/result.hh>

namespace {

    using kernel_ptr = std::unique_ptr<ggg::Kernel>;
    using constructor = kernel_ptr(*)();

    std::array<constructor,size_t(ggg::Protocol::Command::Size)> all_constructors{
        nullptr,
        [] () { return kernel_ptr(new ggg::PAM_kernel); }
    };

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
        std::cerr << err.what() << std::endl;
    }
    {
        Frame frame;
        frame.command = Command::Result;
        auto old_position = out.position();
        out.bump(sizeof(frame));
        Result ret;
        ret.code(kernel->result());
        ret.write(out);
        auto new_position = out.position();
        frame.size = new_position - old_position;
        out.position(old_position);
        out.write(&frame, sizeof(frame));
        out.position(new_position);
    }
}

sys::u32
ggg::Client_protocol::process(Kernel* kernel) {
    using namespace ggg;
    using std::this_thread::yield;
    sys::socket_address address(GGG_BIND_ADDRESS);
    sys::socket s(sys::family_type::unix);
    s.setopt(sys::socket::pass_credentials);
    s.connect(address);
    sys::byte_buffer buf{4096};
    {
        Protocol::Frame frame;
        frame.command = Protocol::Command::PAM_kernel;
        auto old_position = buf.position();
        buf.bump(sizeof(frame));
        kernel->write(buf);
        auto new_position = buf.position();
        frame.size = new_position - old_position;
        buf.position(old_position);
        buf.write(&frame, sizeof(frame));
        buf.position(new_position);
        buf.flip();
        while (buf.remaining() != 0) { buf.flush(s); yield(); }
    }
    buf.clear();
    Result result;
    {
        while (buf.position() < sizeof(sys::u32)) { buf.fill(s); yield(); }
        auto frame = reinterpret_cast<Protocol::Frame*>(buf.data());
        while (buf.position() < frame->size) { buf.fill(s); yield(); }
        buf.flip();
        buf.read(frame, sizeof(Protocol::Frame));
        result.read(buf);
    }
    return result.code();
}
