#include <unistd.h>
#include <iostream>

#include "config.hh"
#include "command.hh"
#include <cstring>

void
usage() {
	std::cerr << "usage: " GGG_EXECUTABLE_NAME " COMMAND [-h]\n";
	std::exit(EXIT_SUCCESS);
}

void
process_command(ggg::Command cmd) {
	using ggg::Command;
	switch (cmd) {
		case Command::Add:
			break;
		case Command::Delete:
			break;
		case Command::Help:
			usage();
			break;
		default:
			usage();
			break;
	}
}

ggg::Command
parse_command(int argc, char* argv[]) {
	using ggg::Command;
	Command cmd;
	const char* str = argv[1];
	if (argc >= 1 && str && str[0] != '-') {
		try {
			cmd = ggg::command_from_string(str);
			::optind = 2;
		} catch (...) {
			std::cerr << "unknown command \"" << str << "\"" << std::endl;
			cmd = Command::Help;
		}
	} else {
		cmd = Command::Help;
	}
	return cmd;
}

int main(int argc, char* argv[]) {
	using ggg::Command;
	Command cmd = parse_command(argc, argv);
	if (argc >= ::optind) {
		int opt;
		while ((opt = ::getopt(argc, argv, "h")) != -1) {
			switch (opt) {
				case 'h':
					cmd = Command::Help;
					break;
			}
		}
	}
	process_command(cmd);
	return EXIT_SUCCESS;
}
