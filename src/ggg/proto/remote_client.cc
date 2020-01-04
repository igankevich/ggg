#include <unistdx/base/log_message>

#include <ggg/proto/protocol.hh>
#include <ggg/proto/remote_client.hh>

void
ggg::Remote_client::process(const sys::epoll_event& event) {
    if (starting() && !event.bad()) {
        sys::log_message("server", "accept _", this->_address);
        this->state(State::Started);
    }
    if (started() && event.bad()) {
        sys::log_message("server", "stopping _", this->_address);
        this->state(State::Stopping);
    }
    if (started()) {
        sys::log_message("server", "started _", this->_address);
        if (event.in()) {
            this->_in.fill(this->_socket);
            this->_in.flip();
            this->_protocol.process(this->_socket, this->_in, this->_out);
            this->_in.compact();
            this->_out.flip();
            this->_out.flush(this->_socket);
            this->_out.compact();
        }
        if (event.out()) {
            this->_out.flip();
            this->_out.flush(this->_socket);
            this->_out.compact();
        }
    }
}
