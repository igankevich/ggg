#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/cli/expire_entity.hh>

void
ggg::Expire_entity::execute()  {
	Database db(Database::File::Entities, Database::Flag::Read_write);
	for (const auto& name : this->args()) {
		db.expire(name.data());
	}
}

void
ggg::Expire_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}


