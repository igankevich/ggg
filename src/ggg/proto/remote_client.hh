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
        Remote_client(sys::socket&& socket, const sys::socket_address& address):
        _address(address) {
            this->_socket = std::move(socket);
        }

        void process(const sys::epoll_event& event) override;

        template <class ... Args>
        inline void
        log(const char* message, const Args& ... args) const {
            sys::log_message("server", message, args...);
        }

    };

}

#endif // vim:filetype=cpp
