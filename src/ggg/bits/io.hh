#ifndef BITS_THROW_IO_ERROR_HH
#define BITS_THROW_IO_ERROR_HH

#include <istream>
#include <ostream>
#include <system_error>
#include <fstream>
#include <unistd.h>

namespace ggg {

	namespace bits {

		template <class Ch>
		inline void
		throw_io_error(std::basic_istream<Ch>& in, const char* msg) {
			if (!in.eof()) {
				throw std::system_error(std::io_errc::stream, msg);
			}
		}

		template <class Ch>
		inline void
		throw_io_error(std::basic_ostream<Ch>& out, const char* msg) {
			throw std::system_error(std::io_errc::stream, msg);
		}

		inline void
		check(const char* path, int mode, bool must_exist=true) {
			if (-1 == ::eaccess(path, mode)) {
				if ((errno == ENOENT && must_exist) || errno != ENOENT) {
					throw std::system_error(errno, std::system_category());
				}
			}
		}

		inline void
		check(const std::string& path, int mode, bool must_exist=true) {
			check(path.data(), mode, must_exist);
		}

		inline void
		rename(const char* old, const char* new_name) {
			int ret = std::rename(old, new_name);
			if (ret == -1) {
				throw std::system_error(errno, std::system_category());
			}
		}

		inline void
		rename(const std::string& old, const std::string& new_name) {
			rename(old.data(), new_name.data());
		}

		template <class T>
		void
		append(const T& ent, const char* dest, const char* msg) {
			typedef typename T::char_type char_type;
			std::basic_ofstream<char_type> out;
			try {
				out.exceptions(std::ios::badbit);
				out.imbue(std::locale::classic());
				out.open(dest, std::ios::out | std::ios::app);
				out << ent << '\n';
			} catch (...) {
				throw_io_error(out, msg);
			}
		}

	}

}

#endif // BITS_THROW_IO_ERROR_HH
