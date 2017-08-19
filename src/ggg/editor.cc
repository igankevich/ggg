#include "editor.hh"

#include <cstdlib>
#include <stdexcept>
#include <unistdx/io/pipe>
#include <unistdx/io/fdstream>
#include <unistdx/ipc/process>
#include <unistdx/ipc/execute>
#include <unistdx/fs/idirtree>

#include "config.hh"

std::string
ggg::find_file_editor() {
	std::string result;
	if (const char* editor = std::getenv("EDITOR")) {
		result = editor;
	} else {
		result = GGG_FILE_EDITOR;
	}
	return result;
}

sys::proc_status
ggg::edit_file(std::string path) {
	std::string editor = find_file_editor();
	sys::process child([&editor,&path] () {
		return sys::this_process::execute_command(editor, path);
	});
	return child.wait();
}

void
ggg::edit_file_or_throw(std::string path) {
	sys::proc_status status = ::ggg::edit_file(path);
	if (!(status.exited() && status.exit_code() == 0)) {
		throw std::runtime_error("bad exit code from editor");
	}
}

sys::proc_status
ggg::edit_directory(sys::path root) {
	sys::pipe pipe;
	pipe.in().unsetf(sys::fildes::non_blocking);
	pipe.out().unsetf(sys::fildes::non_blocking);
	sys::process child([&pipe] () {
		pipe.in().remap(STDIN_FILENO);
		pipe.out().close();
		using namespace sys::this_process;
		return execute_command(GGG_DIRECTORY_EDITOR, '-');
	});
	pipe.in().close();
	{
		sys::ofdstream out(std::move(pipe.out()));
		sys::idirtree tree(root);
		std::copy(
			sys::idirtree_iterator<sys::pathentry>(tree),
			sys::idirtree_iterator<sys::pathentry>(),
			std::ostream_iterator<sys::pathentry>(out, "\n")
		);
		out.flush();
	}
	return child.wait();
}

