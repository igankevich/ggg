#include "command.hh"

#include <iostream>

#include "config.hh"

void
ggg::Command::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << '\n';
}
