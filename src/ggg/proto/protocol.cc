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
            if ((this->_log_pam && frame.command == Command::PAM_kernel) ||
                (this->_log_nss && frame.command == Command::NSS_kernel)) {
                kernel->log_request();
            }
            // TODO remove this when updates are also done through the daemon
            this->_entities.close();
            this->_entities.open(Database::File::Entities, Database::Flag::Read_only);
            this->_accounts.close();
            this->_accounts.open(Database::File::Accounts, Database::Flag::Read_write);
            kernel->run(this->_entities, this->_accounts);
        } catch (const std::exception& err) {
            kernel->result(-1);
            log("kernel read/run error: _", err.what());
        }
        if ((this->_log_pam && frame.command == Command::PAM_kernel) ||
            (this->_log_nss && frame.command == Command::NSS_kernel)) {
            kernel->log_response();
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
    sys::socket s(sys::family_type::unix);
    s.set(sys::socket::options::reuse_address);
    s.set(sys::socket::options::pass_credentials);
    s.connect(this->_server_socket_address);
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
    }
    No_lock lock;
    auto deadline = system_clock::now() + seconds(30);
    sys::event_poller poller;
    poller.emplace(s.fd(), sys::event::inout);
    std::cv_status status;
    enum { Writing, Reading, Finish } state = Writing;
    buf.flip();
    buf.flush(s);
    buf.compact();
    if (buf.position() == 0) { buf.clear(); state = Reading; }
    while (state != Finish) {
        status = poller.wait_until(lock, deadline);
        if (status == std::cv_status::timeout) {
            throw std::runtime_error("timed out");
        }
        for (const auto& event : poller) {
            if (event.bad()) {
                throw std::runtime_error("i/o error");
            }
        }
        if (state == Writing) {
            buf.flip();
            buf.flush(s);
            buf.compact();
            if (buf.position() == 0) { buf.clear(); state = Reading; }
        }
        if (state == Reading) {
            buf.fill(s);
            if (buf.position() < sizeof(Frame)) { continue; }
            Frame frame{};
            std::memcpy(&frame, buf.data(), sizeof(Frame));
            if (buf.position() < frame.size) { continue; }
            buf.flip();
            buf.read(&frame, sizeof(Frame));
            kernel->read(buf);
            buf.compact();
            state = Finish;
        }
    }
    //s.shutdown(sys::shutdown_flag::read_write);
    s.close();
    if (kernel->failed()) {
        throw std::runtime_error("server error");
    }
}
