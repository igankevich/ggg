#ifndef GGG_EDITOR_HH
#define GGG_EDITOR_HH

#include <string>
#include <stdx/debug.hh>
#include <sys/process.hh>
#include <sys/path.hh>

namespace ggg {

	std::string
	find_file_editor();

	sys::proc_status
	edit_file(std::string path);

	sys::proc_status
	edit_directory(sys::path root);

}

#endif // GGG_EDITOR_HH
