#include "expire_entity.hh"

#include <iostream>
#include <algorithm>
#include <string>

#include "config.hh"
#include "ctl/ggg.hh"

void
ggg::Expire_entity::execute()  {
	GGG g(GGG_ENT_ROOT, this->verbose());
	std::for_each(
		this->args_begin(),
		this->args_end(),
		[&] (const std::string& user) {
			g.expire(user);
		}
	);
}

void
ggg::Expire_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}


