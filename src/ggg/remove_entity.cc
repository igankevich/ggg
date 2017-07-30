#include "remove_entity.hh"

#include <iostream>
#include <unistd.h>

#include "config.hh"
#include "ggg.hh"

void
ggg::Remove_entity::execute(int argc, char* argv[])  {
	bool verbose = false;
	int opt;
	while ((opt = getopt(argc, argv, "v")) != -1) {
		if (opt == 'v') {
			verbose = true;
		}
	}
	const char* user = argv[::optind];
	Ggg g(GGG_ROOT, verbose);
	g.erase(user);
}

void
ggg::Remove_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-v] ENTITY\n";
}


