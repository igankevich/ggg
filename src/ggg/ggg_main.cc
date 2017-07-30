#include <unistd.h>
#include <iostream>
#include <stdexcept>

#include "config.hh"
#include "command.hh"
#include "ggg.hh"

void
usage(ggg::Command cmd = ggg::Command::Help) {
	switch (cmd) {
		case ggg::Command::Help:
			std::cerr << "usage: " GGG_EXECUTABLE_NAME " COMMAND [-h]\n";
			break;
		case ggg::Command::Delete:
			std::cerr << "usage: " GGG_EXECUTABLE_NAME " delete USER\n";
			break;
		default:
			usage(ggg::Command::Help);
			break;
	}
	std::exit(EXIT_SUCCESS);
}

void
show_error(const std::exception& err) {
	std::cerr << "error: " << err.what() << std::endl;
}

void
delete_user(const char* user) {
	ggg::Ggg g;
	g.erase(user);
}

void
process_command(ggg::Command cmd, char* argv[]) {
	using ggg::Command;
	switch (cmd) {
		case Command::Add:
			break;
		case Command::Delete:
			delete_user(argv[0]);
			break;
		case Command::Help:
			usage();
			break;
		case Command::Version:
			std::cout << GGG_VERSION "\n";
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
			std::cerr << "error: unknown command \"" << str << "\"" << std::endl;
			cmd = Command::Help;
		}
	} else {
		int opt;
		while ((opt = ::getopt(argc, argv, "h")) != -1) {
			switch (opt) {
				case 'h':
					cmd = Command::Help;
					break;
				case 'v':
					cmd = Command::Version;
					break;
			}
		}
	}
	return cmd;
}

int main(int argc, char* argv[]) {
	using ggg::Command;
	Command cmd = parse_command(argc, argv);
	try {
		process_command(cmd, argv + ::optind);
	} catch (const std::invalid_argument& err) {
		show_error(err);
		usage(cmd);
	} catch (const std::exception& err) {
		show_error(err);
	}
	return EXIT_SUCCESS;
}
