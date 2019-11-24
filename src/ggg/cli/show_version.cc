#include "show_version.hh"

#include <iostream>
#include <unistd.h>

#include <ggg/config.hh>

void
ggg::Show_version::execute()  {
	std::cout << GGG_VERSION << '\n';
}

