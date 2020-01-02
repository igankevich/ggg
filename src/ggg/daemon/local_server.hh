#ifndef GGG_DAEMON_LOCAL_SERVER_HH
#define GGG_DAEMON_LOCAL_SERVER_HH

#include <unistdx/net/socket_address>

#include <ggg/daemon/connection.hh>

namespace ggg {

	class Local_server: public Connection {

	private:
		sys::socket_address _address;

	public:
		explicit Local_server(const sys::socket_address& address);
		inline sys::fd_type fd() const { return this->_socket.fd(); }
		inline sys::port_type port() const { return this->_address.port(); }
		void process(const sys::epoll_event& event) override;

	};

}

#endif // vim:filetype=cpp
