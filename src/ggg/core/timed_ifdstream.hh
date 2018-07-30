#ifndef GGG_CORE_TIMED_IFDSTREAM_HH
#define GGG_CORE_TIMED_IFDSTREAM_HH

#include <chrono>
#include <stdexcept>

#include <unistdx/io/fdstream>
#include <unistdx/io/poll_event>

namespace ggg {

	namespace core {

		class timed_out: public std::exception {};

//		template <class Ch, class Tr=std::char_traits<char>>
		class timed_fildes: public sys::fildes {

		public:
			typedef std::chrono::milliseconds duration;

		private:
			typedef sys::streambuf_traits<sys::fildes> fildes_traits;

		private:
			duration _timeout = std::milliseconds(100);

		public:

			inline explicit
			timed_fildes(sys::fd_type fd):
			sys::fildes(fd)
			{}

			timed_fildes(const timed_fildes&) = delete;

			timed_fildes&
			operator=(const timed_fildes&) = delete;

			timed_fildes(timed_fildes&&) = default;

			timed_fildes&
			operator=(timed_fildes&&) = default;

			inline ssize_t
			read(void* buf, size_t n) const {
				using std::chrono::duration_cast;
				using std::chrono::milliseconds;
				sys::poll_event ev{this->_fd, sys::poll_event::In};
				int ret = ::poll(
					&ev,
					1,
					duration_cast<milliseconds>(this->_timeout)
				);
				if (ret == 0) {
					throw timed_out;
				}
				std::streamsize navail = fildes_traits::in_avail(*this);
				if (navail <= 0) {
					return 0;
				}
				ssize_t ret = ::read(this->_fd, buf, std::min(n, size_t(navail)));
				UNISTDX_CHECK_IO(ret);
				return ret;
			}

		};

	}

}

#endif // vim:filetype=cpp
