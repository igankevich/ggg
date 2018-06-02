#include "editor.hh"

#include <cstdlib>
#include <stdexcept>
#include <unistdx/io/pipe>
#include <unistdx/io/fdstream>
#include <unistdx/ipc/process>
#include <unistdx/ipc/execute>
#include <unistdx/fs/idirtree>

#include <ggg/config.hh>

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
		sys::argstream args;
		args.append(editor);
		args.append(path);
		return sys::this_process::execute_command(args);
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
	pipe.in().unsetf(sys::open_flag::non_blocking);
	pipe.out().unsetf(sys::open_flag::non_blocking);
	sys::process child([&pipe] () {
		pipe.in().remap(STDIN_FILENO);
		pipe.out().close();
		using namespace sys::this_process;
		sys::argstream args;
		args.append(GGG_DIRECTORY_EDITOR);
		args.append('-');
		return execute_command(args);
	});
	pipe.in().close();
	{
		sys::ofdstream out(std::move(pipe.out()));
		sys::idirtree tree(root);
		std::transform(
			sys::idirtree_iterator<sys::direntry>(tree),
			sys::idirtree_iterator<sys::direntry>(),
			std::ostream_iterator<sys::path>(out, "\n"),
			[&tree] (const sys::direntry& entry) {
				return sys::path(tree.current_dir(), entry.name());
			}
		);
		out.flush();
	}
	return child.wait();
}

