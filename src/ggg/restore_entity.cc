#include "restore_entity.hh"

#include <iostream>
#include <algorithm>
#include <string>

#include "config.hh"
#include "ctl/ggg.hh"

void
ggg::Restore_entity::execute()  {
	GGG g(GGG_ROOT, this->verbose());
	std::for_each(
		this->args_begin(),
		this->args_end(),
		[&] (const std::string& user) {
			g.activate(user);
		}
	);
}

void
ggg::Restore_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}



