#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>
#include <ggg/cli/reset_password.hh>

void
ggg::Reset_password::execute()  {
	Database db(GGG_ENTITIES_PATH, false);
	for (const auto& name : this->args()) {
		try {
			db.expire_password(name.data());
		} catch (const std::exception& err) {
			ggg::native_message(std::cerr, "Failed to expire password for _: _", name, err.what());
		}
	}
}

void
ggg::Reset_password::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-q] ENTITY...\n";
}



