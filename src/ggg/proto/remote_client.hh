#ifndef GGG_PROTO_REMOTE_CLIENT_HH
#define GGG_PROTO_REMOTE_CLIENT_HH

#include <unistdx/base/byte_buffer>
#include <unistdx/io/epoll_event>
#include <unistdx/net/socket>
#include <unistdx/net/socket_address>

#include <ggg/proto/connection.hh>
#include <ggg/proto/protocol.hh>

namespace ggg {

    class Remote_client: public Connection {

    private:
        sys::socket_address _address;
        sys::byte_buffer _in{4096}, _out{4096};
        Server_protocol _protocol;

    public:

        inline explicit
        Remote_client(sys::socket& server_socket) {
            server_socket.accept(this->_socket, this->_address);
        }

        void process(const sys::epoll_event& event) override;

    };

}

#endif // vim:filetype=cpp
