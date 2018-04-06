#ifndef BITS_THROW_IO_ERROR_HH
#define BITS_THROW_IO_ERROR_HH

#include <algorithm>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <system_error>

#include <unistdx/base/check>
#include <unistdx/fs/canonical_path>
#include <unistdx/fs/file_mode>
#include <unistdx/fs/path>
#include <unistdx/ipc/identity>

#include <ggg/bits/to_bytes.hh>
#include <ggg/core/eq_traits.hh>

namespace ggg {

	inline void
	set_mode(const char* filename, sys::file_mode m) {
		UNISTDX_CHECK(::chmod(filename, m));
	}

	inline void
	set_owner(const char* filename, sys::uid_type uid, sys::gid_type gid) {
		UNISTDX_CHECK(::chown(filename, uid, gid));
	}

	inline void
	set_perms(
		const char* filename,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m
	) {
		set_owner(filename, uid, gid);
		set_mode(filename, m);
	}

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

		template <class Stream>
		inline void
		open(
			Stream& file,
			std::ios_base::openmode mode,
			sys::path origin,
			sys::uid_type uid,
			sys::file_mode m
		) {
			file.exceptions(std::ios::badbit);
			file.imbue(std::locale::classic());
			file.open(origin, mode | std::ios_base::binary);
			set_mode(origin, m);
		}

		template <class Stream>
		inline void
		open(
			Stream& file,
			std::ios_base::openmode mode,
			sys::path origin,
			sys::uid_type uid
		) {
			open<Stream>(file, mode, origin, uid, uid == 0 ? 0 : 0600);
		}

		template <class Ch>
		inline std::pair<sys::canonical_path,sys::path>
		open_both(
			std::basic_ifstream<Ch>& in,
			std::basic_ofstream<Ch>& out,
			sys::path orig,
			sys::file_mode m
		) {
			sys::canonical_path origin(orig);
			sys::path new_origin;
			new_origin.append(origin);
			new_origin.append(".new");
			check(origin, R_OK | W_OK);
			check(new_origin, R_OK | W_OK, false);
			const sys::uid_type uid = sys::this_process::user();
			open(in, std::ios_base::in, origin, uid, m);
			open(out, std::ios_base::out, new_origin, uid, m);
			return std::make_pair(std::move(origin), std::move(new_origin));
		}

		template <class T>
		void
		append(
			const T& ent,
			const char* dest,
			const char* msg,
			sys::file_mode m
		) {
			typedef typename T::char_type char_type;
			std::basic_ofstream<char_type> out;
			try {
				const sys::uid_type uid = sys::this_process::user();
				open(out, std::ios::out | std::ios::app, sys::path(dest), uid, m);
				out << ent << '\n';
			} catch (...) {
				throw_io_error(out, msg);
			}
		}

		template <class T, class Ch>
		void
		erase(const T& ent, bool verbose, sys::file_mode m) {
			typedef std::istream_iterator<T,Ch> iterator;
			typedef std::ostream_iterator<T,Ch> oiterator;
			std::basic_ifstream<Ch> file;
			std::basic_ofstream<Ch> file_new;
			try {
				auto files = open_both(file, file_new, ent.origin(), m);
				std::copy_if(
					iterator(file),
					iterator(),
					oiterator(file_new, delimiter_traits<Ch>::newline()),
					[&ent,verbose] (const T& rhs) {
						const bool match = rhs.name() == ent.name();
						if (match && verbose) {
							log_traits<Ch>::log()
							<< "removing " << rhs.name()
							<< " from " << ent.origin()
							<< std::endl;
						}
						return !match;
					}
				);
				rename(files.second, files.first);
			} catch (const std::exception& err) {
				throw_io_error(file, err.what());
			} catch (...) {
				wcvt_type cv;
				std::stringstream msg;
				msg << "unable to remove " << to_bytes<char>(cv, ent.name());
				throw_io_error(file, msg.str());
			}
		}

		template <class T, class Ch>
		void
		update(const T& ent, bool verbose, sys::file_mode m) {
			typedef std::istream_iterator<T,Ch> iterator;
			typedef std::ostream_iterator<T,Ch> oiterator;
			typedef ::ggg::eq_traits<T> traits_type;
			std::basic_ifstream<Ch> file;
			std::basic_ofstream<Ch> file_new;
			try {
				auto files = open_both(file, file_new, ent.origin(), m);
				std::transform(
					iterator(file),
					iterator(),
					oiterator(file_new, bits::delimiter_traits<Ch>::newline()),
					[&ent,verbose] (const T& rhs) {
						const bool match = traits_type::eq(rhs, ent);
						if (match && verbose) {
							log_traits<Ch>::log()
							<< "modifying " << rhs.name()
							<< " from " << ent.origin()
							<< std::endl;
						}
						return match ? ent : rhs;
					}
				);
				rename(files.second, files.first);
			} catch (const std::exception& err) {
				throw_io_error(file, err.what());
			} catch (...) {
				wcvt_type cv;
				std::stringstream msg;
				msg << "unable to modify " << to_bytes<char>(cv, ent.name());
				throw_io_error(file, msg.str());
			}
		}

	}

}

#endif // BITS_THROW_IO_ERROR_HH
