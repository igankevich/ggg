#ifndef GGG_EDITOR_HH
#define GGG_EDITOR_HH

#include <string>
#include <unistdx/ipc/proc_status>
#include <unistdx/fs/path>

namespace ggg {

	std::string
	find_file_editor();

	sys::proc_status
	edit_file(std::string path);

	void
	edit_file_or_throw(std::string path);

	sys::proc_status
	edit_directory(sys::path root);

}

#endif // GGG_EDITOR_HH
