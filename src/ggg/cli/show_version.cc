#include "show_version.hh"

#include <iostream>
#include <unistd.h>

#include <ggg/config.hh>

#define GGG_REVISION GGG_VERSION
#define GGG_BUILD_DATE GGG_VERSION

void
ggg::Show_version::parse_arguments(int argc, char* argv[])  {
	int opt;
	while ((opt = getopt(argc, argv, "rd")) != -1) {
		if (opt == 'r') {
			this->_field = Field::Revision;
		} else if (opt == 'd') {
			this->_field = Field::Date;
		}
	}
}

void
ggg::Show_version::execute()  {
	const char* field;
	switch (this->_field) {
		case Field::Version:
			field = GGG_VERSION;
			break;
		case Field::Revision:
			field = GGG_REVISION;
			break;
		case Field::Date:
			field = GGG_BUILD_DATE;
			break;
		default:
			field = "unknown";
			break;
	}
	std::cout << field << '\n';
}

void
ggg::Show_version::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-rd] COMMAND" << '\n';
}

