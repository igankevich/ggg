#ifndef GGG_CLI_EDITOR_HH
#define GGG_CLI_EDITOR_HH

#include <string>
#include <unistdx/ipc/process_status>
#include <unistdx/fs/path>

namespace ggg {

	std::string
	find_file_editor();

	sys::process_status
	edit_file(std::string path);

	void
	edit_file_or_throw(std::string path);

}

#endif // vim:filetype=cpp
