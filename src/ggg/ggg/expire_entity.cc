#include "expire_entity.hh"

#include <iostream>
#include <algorithm>
#include <string>

#include <ggg/config.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/core/lock.hh>

void
ggg::Expire_entity::execute()  {
	Database db(GGG_DATABASE_PATH, false);
	for (const auto& name : this->args()) {
		db.expire(name.data());
	}
}

void
ggg::Expire_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}


