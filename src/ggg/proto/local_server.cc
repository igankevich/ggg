#include <ggg/proto/local_server.hh>
#include <ggg/proto/pipeline.hh>
#include <ggg/proto/remote_client.hh>

ggg::Local_server::Local_server(const sys::socket_address& address):
Network_connection(address.family()), _address(address) {
    this->_socket.set(sys::socket::options::reuse_address);
    this->_socket.bind(this->_address);
    this->_socket.listen();
    log("listen _", this->_address);
}

void
ggg::Local_server::process(const sys::epoll_event& event) {
    Network_connection::process(event);
    if (started() && event.in()) {
        sys::socket client_socket;
        sys::socket_address client_address;
        while (this->_socket.accept(client_socket, client_address)) {
            //log("add client fd _ address _", client_socket.fd(), client_address);
            this->parent()->add(
                new Remote_client(std::move(client_socket), client_address),
                sys::event::inout);
        }
    }
}
