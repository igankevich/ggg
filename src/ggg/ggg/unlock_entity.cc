#include <ggg/ggg/unlock_entity.hh>

#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Unlock_entity::execute()  {
	Database db(GGG_DATABASE_PATH, false);
	for (const auto& name : this->args()) {
		try {
			db.activate(name.data());
		} catch (const std::exception& err) {
			ggg::native_message(std::cerr, "Failed to unlock _: _", name, err.what());
		}
	}
}

void
ggg::Unlock_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}



