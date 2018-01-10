#include "reset_password.hh"

#include <iostream>
#include <algorithm>
#include <string>

#include <ggg/config.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/core/lock.hh>

void
ggg::Reset_password::execute()  {
	file_lock lock(true);
	GGG g(GGG_ENT_ROOT, this->verbose());
	std::for_each(
		this->args_begin(),
		this->args_end(),
		[&] (const std::string& user) {
			g.reset(user);
		}
	);
}

void
ggg::Reset_password::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}



