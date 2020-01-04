#include <unistdx/base/log_message>

#include <ggg/proto/local_server.hh>
#include <ggg/proto/pipeline.hh>
#include <ggg/proto/remote_client.hh>

ggg::Local_server::Local_server(const sys::socket_address& address):
Connection(address.family()), _address(address) {
    this->_socket.setopt(sys::socket::reuse_addr);
    this->_socket.bind(this->_address);
    this->_socket.listen();
    sys::log_message("server", "listen _", this->_address);
}

void
ggg::Local_server::process(const sys::epoll_event& event) {
    Connection::process(event);
    if (started() && event.in()) {
        this->parent()->add(new Remote_client(this->_socket), sys::event::inout);
    }
}
