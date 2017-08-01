#include "command.hh"

#include <iostream>
#include <unistd.h>

#include "config.hh"

void
ggg::Command::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "q")) != -1) {
		if (opt == 'q') {
			this->_verbose = false;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace(argv[i]);
	}
}

void
ggg::Command::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << '\n';
}
