#ifndef GGG_TMPFILE_HH
#define GGG_TMPFILE_HH

#include <sys/fildesbuf.hh>
#include <sys/path.hh>
#include <string>

namespace sys {

	sys::path
	find_temporary_directory();

	std::string
	get_temporary_file_name_template(sys::path dir);

	class tmpfile {
		std::string _filename;
		sys::ofdstream _out;

	public:
		tmpfile();
		tmpfile(tmpfile&&) = default;
		~tmpfile();
		tmpfile(const tmpfile&) = delete;
		tmpfile& operator=(const tmpfile&) = delete;

		inline std::ostream&
		out() noexcept {
			return this->_out;
		}

		inline const std::string&
		filename() const noexcept {
			return this->_filename;
		}

	};

}

#endif // GGG_TMPFILE_HH
