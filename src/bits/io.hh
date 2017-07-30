#ifndef BITS_THROW_IO_ERROR_HH
#define BITS_THROW_IO_ERROR_HH

#include <istream>
#include <system_error>
#include <unistd.h>

namespace ggg {

	namespace bits {

		inline void
		throw_io_error(std::istream& in, const char* msg) {
			if (!in.eof()) {
				throw std::system_error(std::io_errc::stream, msg);
			}
		}

		inline void
		check(const char* path, int mode, bool must_exist=true) {
			if (-1 == ::access(path, mode)) {
				if ((errno == ENOENT && must_exist) || errno != ENOENT) {
					throw std::system_error(errno, std::system_category());
				}
			}
		}

		inline void
		rename(const char* old, const char* new_name) {
			int ret = std::rename(old, new_name);
			if (ret == -1) {
				throw std::system_error(errno, std::system_category());
			}
		}

	}

}

#endif // BITS_THROW_IO_ERROR_HH
