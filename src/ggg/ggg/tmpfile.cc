#include "tmpfile.hh"

#include <array>
#include <algorithm>
#include <vector>
#include <stdexcept>

#include <unistdx/fs/file_stat>

#include <ggg/config.hh>
#include <ggg/core/fs.hh>

namespace {

	std::array<std::string,3> all_tmpdirs{"TMPDIR", "TMP", "TEMP"};

	inline sys::fildes
	get_fd(std::string& tpl) {
		sys::fildes fd(::mkostemp(&tpl[0], O_CLOEXEC));
		if (!fd) {
			throw std::runtime_error(
				"unable to create a file in temporary directory"
			);
		}
		return fd;
	}

}

sys::path
sys::find_temporary_directory() {
	std::string tmp;
	std::vector<std::string> possible_dirs;
	std::for_each(
		all_tmpdirs.begin(),
		all_tmpdirs.end(),
		[&possible_dirs] (const std::string& envvar) {
			if (const char* dir = std::getenv(envvar.data())) {
				possible_dirs.emplace_back(dir); }
		}
	);
	possible_dirs.emplace_back("/dev/shm");
	possible_dirs.emplace_back("/tmp");
	auto result = std::find_if(
		possible_dirs.begin(),
		possible_dirs.end(),
		[] (const std::string& rhs) {
			sys::file_stat st(rhs.data());
			return st.is_directory() && ggg::can_write(sys::path(rhs));
		}
	);
	if (result == possible_dirs.end()) {
		throw std::runtime_error("unable to find temporary directory");
	}
	return sys::path(*result);
}

std::string
sys::get_temporary_file_name_template(sys::path dir) {
	std::string basename;
	basename.append(GGG_EXECUTABLE_NAME);
	basename.push_back('.');
	basename.append("XXXXXX");
	return sys::path(dir, basename);
}

sys::tmpfile::tmpfile():
_filename(get_temporary_file_name_template(find_temporary_directory())),
_out(get_fd(_filename))
{}

sys::tmpfile::~tmpfile() {
	//TODO
	//this->_out.close();
	(void)std::remove(this->_filename.data());
}
