#include <unistdx/base/log_message>

#include <ggg/proto/protocol.hh>
#include <ggg/proto/remote_client.hh>

void
ggg::Remote_client::process(const sys::epoll_event& event) {
    Connection::process(event);
    if (started()) {
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
