#include "remove_entity.hh"

#include <iostream>
#include <unordered_set>
#include <string>
#include <tuple>
#include <unistd.h>

#include <ggg/config.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/core/lock.hh>

void
ggg::Remove_entity::execute()  {
	file_lock lock(true);
	GGG g(GGG_ENT_ROOT, this->verbose());
	std::for_each(
		this->args_begin(),
		this->args_end(),
		[&] (const std::string& user) {
			g.erase(user);
		}
	);
}

void
ggg::Remove_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}


