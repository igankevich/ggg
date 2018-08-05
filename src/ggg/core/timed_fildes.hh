#ifndef GGG_CORE_TIMED_IFDSTREAM_HH
#define GGG_CORE_TIMED_IFDSTREAM_HH

#include <chrono>
#include <stdexcept>

#include <unistdx/io/fdstream>
#include <unistdx/io/fildes>
#include <unistdx/io/poll_event>

namespace ggg {

	namespace core {

		class timed_out: public std::exception {};

//		template <class Ch, class Tr=std::char_traits<char>>
		class timed_fildes: public sys::fildes {

		public:
			typedef std::chrono::system_clock clock_type;
			typedef clock_type::duration duration;
			typedef char char_type;

		private:
			typedef sys::streambuf_traits<sys::fildes> fildes_traits;

		private:
			duration _timeout = std::chrono::milliseconds(100);

		public:

			inline explicit
			timed_fildes(sys::fd_type fd):
			sys::fildes(fd)
			{}

			inline
			timed_fildes(
				const char* filename,
				sys::open_flag flags,
				sys::mode_type mode=0
			): sys::fildes(filename, flags, mode)
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
					duration_cast<milliseconds>(this->_timeout).count()
				);
				if (ret == 0) {
					throw timed_out();
				}
				UNISTDX_CHECK(ret);
				ssize_t m = ::read(this->_fd, buf, n);
				UNISTDX_CHECK_IO(m);
				return m;
			}

			inline void
			timeout(duration rhs) noexcept {
				this->_timeout = rhs;
			}

		};

		typedef sys::basic_fdstream<char, std::char_traits<char>, timed_fildes>
			timed_ifdstream;

	}

}

namespace sys {
	template<>
	struct streambuf_traits<::ggg::core::timed_fildes>:
	public fildes_streambuf_traits<::ggg::core::timed_fildes>
	{};
}

#endif // vim:filetype=cpp
