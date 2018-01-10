#include "show_help.hh"

#include <iostream>
#include <unistd.h>

#include <ggg/config.hh>

void
ggg::Show_help::parse_arguments(int argc, char* argv[])  {
	if (*this->prefix().data() != '-' && ::optind < argc) {
		const char* str = argv[::optind];
		if (str && str[0] != '-') {
			try {
				this->_ptr = command_from_string(str);
			} catch (...) {
			}
		}
	}
}

void
ggg::Show_help::execute()  {
	if (this->_ptr) {
		this->_ptr->print_usage();
	} else {
		std::cout << "usage: " GGG_EXECUTABLE_NAME " [-h] COMMAND\n";
	}
}

void
ggg::Show_help::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " COMMAND" << '\n';
}

