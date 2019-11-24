#ifndef GGG_CLI_TMPFILE_HH
#define GGG_CLI_TMPFILE_HH

#include <unistdx/io/fdstream>
#include <unistdx/fs/path>
#include <string>

namespace sys {

	sys::path
	find_temporary_directory();

	std::string
	get_temporary_file_name_template(sys::path dir, std::string suffix);

	class tmpfile {
		std::string _filename;
		sys::ofdstream _out;

	public:
		tmpfile(std::string suffix="");
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

#endif // vim:filetype=cpp
