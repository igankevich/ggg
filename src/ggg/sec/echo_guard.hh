#ifndef GGG_SEC_ECHO_GUARD_HH
#define GGG_SEC_ECHO_GUARD_HH

#include <termios.h>
#include <unistd.h>

#include <unistdx/io/fildes>

namespace ggg {

	class echo_guard: public ::termios {

	private:
		sys::fd_type _fd = -1;

	public:

		inline explicit
		echo_guard(sys::fd_type fd):
		_fd(fd) {
			if (::isatty(this->_fd)) {
				UNISTDX_CHECK(::tcgetattr(this->_fd, this));
				this->c_lflag &= ~ECHO;
				UNISTDX_CHECK(::tcsetattr(this->_fd, TCSANOW, this));
			} else {
				this->_fd = -1;
			}
		}

		inline
		~echo_guard() {
			if (this->_fd != -1) {
				this->c_lflag |= ECHO;
				int ret = ::tcsetattr(this->_fd, TCSANOW, this);
				if (ret == -1) {
					std::terminate();
				}
			}
		}

	};

}

#endif // vim:filetype=cpp
