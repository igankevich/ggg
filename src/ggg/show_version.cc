#include "show_version.hh"

#include <iostream>

#include "config.hh"

void
ggg::Show_version::execute(int argc, char* argv[])  {
	std::cout << GGG_VERSION "\n";
}
