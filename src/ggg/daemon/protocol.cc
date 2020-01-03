#include <array>
#include <memory>

#include <ggg/daemon/authentication.hh>
#include <ggg/daemon/kernel.hh>
#include <ggg/daemon/protocol.hh>

namespace {

    using kernel_ptr = std::unique_ptr<ggg::Kernel>;
	using constructor = kernel_ptr(*)();

    std::array<constructor,size_t(ggg::Protocol::Command::Size)> all_constructors{
        nullptr,
        [] () { return kernel_ptr(new ggg::Authenticate); }
    };

}

void
ggg::Server_protocol::read(byte_buffer& buf) {
    if (buf.position() < sizeof(sys::u32)) { return; }
    auto frame = reinterpret_cast<Frame*>(buf.data());
    if (buf.position() < frame->size) { return; }
    if (frame->command == Command::Unknown) { return; }
    if (frame->command >= Command::Size) { return; }
    auto kernel = all_constructors[size_t(frame->command)]();
    buf.limit(buf.position() + frame->size);
    buf.bump(sizeof(Frame));
    kernel->read(buf);
    kernel->run();
    buf.compact();
}

void
ggg::Server_protocol::write(byte_buffer& buf) {
}
