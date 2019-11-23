#include <iostream>

#include <ggg/cli/find_machines.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>

void
ggg::Find_machines::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
}

void
ggg::Find_machines::execute() {
	Database db(Database::File::Entities);
	this->_result =db.machines();
	Show_base::execute();
}

void
ggg::Find_machines::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [ADDRESS | NAME] ..." << '\n';
}
