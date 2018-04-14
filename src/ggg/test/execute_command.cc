#include "execute_command.hh"

#include <unistdx/io/fdstream>
#include <unistdx/io/pipe>
#include <unistdx/ipc/execute>
#include <unistdx/ipc/process>

std::pair<sys::proc_status,std::string>
execute_command(const char* cmd) {
	sys::pipe pipe;
	pipe.in().unsetf(sys::open_flag::non_blocking);
	pipe.out().unsetf(sys::open_flag::non_blocking);
	sys::process child {
		[cmd,&pipe] () {
			pipe.in().close();
			pipe.out().remap(STDOUT_FILENO);
			return sys::this_process::execute_command("/bin/sh", "-c", cmd);
		}
	};
	pipe.out().close();
	sys::ifdstream in(std::move(pipe.in()));
	std::stringstream tmp;
	tmp << in.rdbuf();
	return std::make_pair(child.wait(), tmp.str());
}

