#include <ggg/cli/lock_entity.hh>

#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Lock_entity::execute()  {
	Database db(GGG_ENTITIES_PATH, false);
	for (const auto& name : this->args()) {
		try {
			db.deactivate(name.data());
		} catch (const std::exception& err) {
			ggg::native_message(std::cerr, "Failed to lock _: _", name, err.what());
		}
	}
}

void
ggg::Lock_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}



