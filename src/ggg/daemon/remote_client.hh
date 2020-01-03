#ifndef GGG_DAEMON_REMOTE_CLIENT_HH
#define GGG_DAEMON_REMOTE_CLIENT_HH

#include <unistdx/io/epoll_event>
#include <unistdx/net/socket>
#include <unistdx/net/socket_address>

#include <ggg/daemon/byte_buffer.hh>
#include <ggg/daemon/connection.hh>
#include <ggg/daemon/protocol.hh>

namespace ggg {

	class Remote_client: public Connection {

	private:
		sys::socket_address _address;
        byte_buffer _buffer{4096};
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
