#ifndef GGG_PROTO_LOCAL_SERVER_HH
#define GGG_PROTO_LOCAL_SERVER_HH

#include <unistdx/base/log_message>
#include <unistdx/net/socket_address>

#include <ggg/proto/connection.hh>

namespace ggg {

    class Local_server: public Connection {

    private:
        sys::socket_address _address;

    public:
        explicit Local_server(const sys::socket_address& address);
        inline sys::fd_type fd() const { return this->_socket.fd(); }
        inline sys::port_type port() const {
            return sys::socket_address_cast<sys::ipv4_socket_address>(this->_address).port();
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
