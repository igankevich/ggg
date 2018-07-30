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
#include <ggg/core/acl.hh>
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

	inline void
	set_acl(const char* filename, const ggg::acl::access_control_list& acl) {
		try {
			acl.add_mask();
			acl.write(filename, ggg::acl::access_acl);
		} catch (...) {
			// ignore
		}
	}

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
			sys::uid_type uid,
			const ggg::acl::access_control_list& acl
		) {
			file.exceptions(std::ios::badbit);
			file.imbue(std::locale::classic());
			file.open(origin, mode | std::ios_base::binary);
			set_acl(origin, acl);
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
			const ggg::acl::access_control_list& acl
		) {
			sys::canonical_path origin(orig);
			sys::path new_origin;
			new_origin.append(origin);
			new_origin.append(".new");
			check(origin, R_OK | W_OK);
			check(new_origin, R_OK | W_OK, false);
			const sys::uid_type uid = sys::this_process::user();
			open(in, std::ios_base::in, origin, uid, acl);
			open(out, std::ios_base::out, new_origin, uid, acl);
			return std::make_pair(std::move(origin), std::move(new_origin));
		}

		template <class Ch>
		inline std::pair<sys::canonical_path,sys::path>
		open_both(
			std::basic_ifstream<Ch>& in,
			std::basic_ofstream<Ch>& out,
			sys::path orig,
			sys::file_mode m
		) {
			return open_both<Ch>(in, out, orig, ggg::acl::access_control_list(m));
		}

		template <class T>
		void
		append(
			const T& ent,
			const char* dest,
			const char* msg,
			const ggg::acl::access_control_list& acl
		) {
			typedef typename T::char_type char_type;
			std::basic_ofstream<char_type> out;
			try {
				const sys::uid_type uid = sys::this_process::user();
				open(out, std::ios::out | std::ios::app, sys::path(dest), uid, acl);
				out << ent << '\n';
			} catch (...) {
				throw_io_error(out, msg);
			}
		}

		template <class T>
		void
		append(
			const T& ent,
			const char* dest,
			const char* msg,
			sys::file_mode m
		) {
			return append<T>(ent, dest, msg, ggg::acl::access_control_list(m));
		}

		template <class T, class Ch>
		void
		erase(
			const T& ent,
			bool verbose,
			const ggg::acl::access_control_list& acl
		) {
			typedef std::istream_iterator<T,Ch> iterator;
			typedef std::ostream_iterator<T,Ch> oiterator;
			std::basic_ifstream<Ch> file;
			std::basic_ofstream<Ch> file_new;
			try {
				auto files = open_both(file, file_new, ent.origin(), acl);
				std::copy_if(
					iterator(file),
					iterator(),
					oiterator(file_new, "\n"),
					[&ent,verbose] (const T& rhs) {
						const bool match = rhs.name() == ent.name();
						if (match && verbose) {
							std::clog << "removing " << rhs.name()
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
				std::stringstream msg;
				msg << "unable to remove " << ent.name();
				throw_io_error(file, msg.str());
			}
		}

		template <class T, class Ch>
		void
		erase(const T& ent, bool verbose, sys::file_mode m) {
			erase<T,Ch>(ent, verbose, ggg::acl::access_control_list(m));
		}

		template <class T, class Ch>
		void
		update(
			const T& ent,
			bool verbose,
			const ggg::acl::access_control_list& acl
		) {
			typedef std::istream_iterator<T,Ch> iterator;
			typedef std::ostream_iterator<T,Ch> oiterator;
			typedef ::ggg::eq_traits<T> traits_type;
			std::basic_ifstream<Ch> file;
			std::basic_ofstream<Ch> file_new;
			try {
				auto files = open_both(file, file_new, ent.origin(), acl);
				std::transform(
					iterator(file),
					iterator(),
					oiterator(file_new, "\n"),
					[&ent,verbose] (const T& rhs) {
						const bool match = traits_type::eq(rhs, ent);
						if (match && verbose) {
							std::clog
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
				std::stringstream msg;
				msg << "unable to modify " << ent.name();
				throw_io_error(file, msg.str());
			}
		}

		template <class T, class Ch>
		void
		update(const T& ent, bool verbose, sys::file_mode m) {
			update<T,Ch>(ent, verbose, ggg::acl::access_control_list(m));
		}

	}

}

#endif // BITS_THROW_IO_ERROR_HH
