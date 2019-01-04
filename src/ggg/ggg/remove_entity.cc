#include "remove_entity.hh"

#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Remove_entity::execute()  {
	Database db(GGG_DATABASE_PATH, false);
	for (const auto& name : this->args()) {
		try {
			db.erase(name.data());
		} catch (const std::exception& err) {
			ggg::native_message(std::cerr, "Failed to remove _: _", name, err.what());
		}
	}
}

void
ggg::Remove_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}


