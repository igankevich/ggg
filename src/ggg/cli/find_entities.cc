#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/cli/find_entities.hh>
#include <ggg/cli/search.hh>

void
ggg::Find_entities::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
}

void
ggg::Find_entities::execute() {
	Database db(Database::File::Entities);
	Search search(&db, this->args_begin(), this->args_end());
	auto rstr = this->args().empty() ? db.entities() : db.search_entities();
	user_iterator first(rstr), last;
	std::move(first, last, std::inserter(this->_result, this->_result.begin()));
	Show_base::execute();
}

void
ggg::Find_entities::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " REGEX..." << '\n';
}
