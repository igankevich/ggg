#include <iostream>

#include <ggg/cli/cli_traits.hh>
#include <ggg/cli/find_entities.hh>
#include <ggg/cli/search.hh>
#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/core/entity.hh>

void
ggg::Find_entities::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "t:o:")) != -1) {
		switch (opt) {
			case 't': std::string(::optarg) >> this->_type; break;
			case 'o': std::string(::optarg) >> this->_oformat; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
}

void
ggg::Find_entities::execute() {
	Database db;
    Search search;
    Database::statement_type st;
	switch (this->_type) {
		case Entity_type::Entity:
			db.open(Database::File::Entities);
            search.open(&db, this->args_begin(), this->args_end());
            st = db.search_entities();
            write<entity>(std::cout, st, this->_oformat);
			break;
		case Entity_type::Account:
			db.open(Database::File::Accounts);
            search.open(&db, this->args_begin(), this->args_end());
            st = db.search_accounts();
            write<account>(std::cout, st, this->_oformat);
			break;
		case Entity_type::Machine:
            throw std::runtime_error("not implemented");
			break;
		case Entity_type::Message:
            throw std::runtime_error("not implemented");
			break;
	}
}

void
ggg::Find_entities::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " REGEX..." << '\n';
}
