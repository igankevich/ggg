#ifndef BITS_THROW_IO_ERROR_HH
#define BITS_THROW_IO_ERROR_HH

#include <algorithm>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <system_error>

#include <unistdx/fs/canonical_path>
#include <unistdx/fs/path>

#include <ggg/bits/to_bytes.hh>

namespace ggg {

	namespace bits {

		template <class Ch>
		struct delimiter_traits;

		template <>
		struct delimiter_traits<char> {

			inline static const char*
			newline() noexcept {
				return "\n";
			}

		};

		template <>
		struct delimiter_traits<wchar_t> {

			inline static const wchar_t*
			newline() noexcept {
				return L"\n";
			}

		};

		template <class Ch>
		struct log_traits;

		template <>
		struct log_traits<char> {
			inline static std::ostream&
			log() noexcept {
				return std::clog;
			}
		};

		template <>
		struct log_traits<wchar_t> {
			inline static std::wostream&
			log() noexcept {
				return std::wclog;
			}
		};

		template <class Ch>
		inline void
		throw_io_error(std::basic_istream<Ch>& in, const char* msg) {
			if (!in.eof()) {
				throw std::system_error(std::io_errc::stream, msg);
			}
		}

		template <class Ch>
		inline void
		throw_io_error(std::basic_istream<Ch>& in, std::string msg) {
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

		/*
		template <class Ch>
		inline void
		open_stream(std::basic_istream<Ch>& file, const sys::path& origin) {
			file.exceptions(std::ios::badbit);
			file.imbue(std::locale::classic());
			file.open(origin, std::ios_base::in | std::ios_base::binary);
		}
		*/

		template <class T, class Ch>
		void
		erase(const T& ent, bool verbose) {
			typedef std::istream_iterator<T,Ch> iterator;
			typedef std::ostream_iterator<T,Ch> oiterator;
			std::string new_origin;
			sys::canonical_path origin(ent.origin());
			new_origin.append(origin);
			new_origin.append(".new");
			wcvt_type cv;
			check(origin, R_OK | W_OK);
			check(new_origin, R_OK | W_OK, false);
			std::basic_ifstream<Ch> file;
			std::basic_ofstream<Ch> file_new;
			try {
				file.exceptions(std::ios::badbit);
				file.imbue(std::locale::classic());
				file.open(origin, std::ios_base::in | std::ios_base::binary);
				file_new.exceptions(std::ios::badbit);
				file_new.imbue(std::locale::classic());
				file_new.open(new_origin, std::ios_base::out | std::ios_base::binary);
				std::copy_if(
					iterator(file),
					iterator(),
					oiterator(file_new, delimiter_traits<Ch>::newline()),
					[&ent,verbose] (const T& rhs) {
						const bool match = rhs.name() == ent.name();
						if (match && verbose) {
							log_traits<Ch>::log()
							<< "removing " << rhs.name() << ':' << rhs.id()
							<< " from " << ent.origin()
							<< std::endl;
						}
						return !match;
					}
				);
			} catch (...) {
				std::stringstream msg;
				msg << "unable to remove " << ent.name();
				throw_io_error(file, msg.str());
			}
			rename(new_origin, origin);
		}

		template <class T, class Ch>
		void
		update(const T& ent, bool verbose) {
			typedef std::istream_iterator<T,Ch> iterator;
			typedef std::ostream_iterator<T,Ch> oiterator;
			std::string new_origin;
			sys::canonical_path origin(ent.origin());
			new_origin.append(origin);
			new_origin.append(".new");
			wcvt_type cv;
			check(origin, R_OK | W_OK);
			check(new_origin, R_OK | W_OK, false);
			std::basic_ifstream<Ch> file;
			std::basic_ofstream<Ch> file_new;
			try {
				file.exceptions(std::ios::badbit);
				file.imbue(std::locale::classic());
				file.open(origin, std::ios_base::in | std::ios_base::binary);
				file_new.exceptions(std::ios::badbit);
				file_new.imbue(std::locale::classic());
				file_new.open(new_origin, std::ios_base::out | std::ios_base::binary);
				std::copy_if(
					iterator(file),
					iterator(),
					oiterator(file_new, bits::delimiter_traits<Ch>::newline()),
					[&ent,verbose] (const T& rhs) {
						const bool match = rhs.name() == ent.name();
						if (match && verbose) {
							log_traits<Ch>::log()
							<< "removing " << rhs.name() << ':' << rhs.id()
							<< " from " << ent.origin()
							<< std::endl;
						}
						return match ? ent : rhs;
					}
				);
			} catch (...) {
				std::stringstream msg;
				msg << "unable to remove " << ent.name();
				throw_io_error(file, msg.str());
			}
			rename(new_origin, origin);
		}

	}

}

#endif // BITS_THROW_IO_ERROR_HH
