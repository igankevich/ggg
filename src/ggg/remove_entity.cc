#include "remove_entity.hh"

#include <iostream>
#include <unordered_set>
#include <string>
#include <tuple>
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
	Ggg g(GGG_ROOT, verbose);
	std::unordered_set<std::string> users;
	for (int i=::optind; i<argc; ++i) {
		const char* user = argv[i];
		bool success;
		std::tie(std::ignore, success) = users.emplace(user);
		if (success) {
			g.erase(user);
		}
	}
}

void
ggg::Remove_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-v] ENTITY...\n";
}


