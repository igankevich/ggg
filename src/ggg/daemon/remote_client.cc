#include <unistdx/base/log_message>

#include <ggg/daemon/protocol.hh>
#include <ggg/daemon/remote_client.hh>

void
ggg::Remote_client::process(const sys::epoll_event& event) {
    if (starting() && !event.bad()) {
        sys::log_message("server", "accept _", this->_address);
        this->state(State::Started);
    }
    if (started() && event.bad()) {
        this->state(State::Stopping);
    }
    if (started()) {
        if (event.in()) {
            this->_buffer.read(this->_socket);
            this->_buffer.flip();
            this->_protocol.read(this->_buffer);
        }
        if (event.out()) {
        }
    }
}

