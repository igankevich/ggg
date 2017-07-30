#include "show_help.hh"

#include <iostream>
#include <unistd.h>

#include "config.hh"
#include "all_commands.hh"

void
ggg::Show_help::execute(int argc, char* argv[])  {
	bool success = false;
	if (*this->prefix().data() != '-' && ::optind < argc) {
		const char* str = argv[::optind];
		if (str && str[0] != '-') {
			try {
				command_ptr ptr = command_from_string(str);
				ptr->print_usage();
				success = true;
			} catch (...) {
			}
		}
	}
	if (!success) {
		std::cout << "usage: " GGG_EXECUTABLE_NAME " [-h] COMMAND\n";
	}
}

void
ggg::Show_help::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " COMMAND" << '\n';
}

