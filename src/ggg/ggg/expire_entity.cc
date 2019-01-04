#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/ggg/expire_entity.hh>

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


