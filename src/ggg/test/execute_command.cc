#include <ggg/test/execute_command.hh>

#include <unistdx/io/fdstream>
#include <unistdx/io/pipe>
#include <unistdx/ipc/execute>
#include <unistdx/ipc/process>

std::tuple<sys::process_status,std::string,std::string>
execute_command(const char* cmd) {
	sys::pipe pipes[2];
    for (auto& p : pipes) {
        p.in().unsetf(sys::open_flag::non_blocking);
        p.out().unsetf(sys::open_flag::non_blocking);
    }
	sys::process child {
		[cmd,&pipes] () {
            for (auto& p : pipes) { p.in().close(); }
			sys::fildes out(STDOUT_FILENO);
			out = pipes[0].out();
			sys::fildes err(STDERR_FILENO);
			err = pipes[1].out();
			sys::argstream args;
			args.append("/bin/sh");
			args.append("-c");
			args.append(cmd);
			return sys::this_process::execute_command(args);
		}
	};
    for (auto& p : pipes) { p.out().close(); }
    std::string output[2];
    for (int i=0; i<2; ++i) {
        sys::ifdstream in(std::move(pipes[i].in()));
        std::stringstream tmp;
        tmp << in.rdbuf();
        output[i] = tmp.str();
    }
	return std::make_tuple(child.wait(), output[0], output[1]);
}


