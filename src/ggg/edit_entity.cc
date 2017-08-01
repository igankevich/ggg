#include "edit_entity.hh"

#include <iostream>
#include <algorithm>
#include <string>
#include <set>
#include <iterator>

#include "config.hh"
#include "ggg.hh"
#include "align_columns.hh"

void
ggg::Edit_entity::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "qae")) != -1) {
		switch (opt) {
			case 'q':
				this->_verbose = false;
				break;
			case 'a':
				this->_type = Type::Account;
				break;
			case 'e':
				this->_type = Type::Entity;
				break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace(argv[i]);
	}
}

void
ggg::Edit_entity::execute()  {
	Ggg g(GGG_ROOT, this->verbose());
	switch (this->_type) {
		case Type::Entity:
			print_entities(g, std::cout);
			break;
		case Type::Account:
			print_accounts(g, std::cout);
			break;
	}
}

void
ggg::Edit_entity::print_entities(Ggg& g, std::ostream& out) {
	std::set<entity> entities;
	g.find_entities(
		this->args_begin(),
		this->args_end(),
		std::inserter(entities, entities.begin())
	);
	align_columns(entities, out, entity::delimiter);
}

void
ggg::Edit_entity::print_accounts(Ggg& g, std::ostream& out) {
	std::set<account> accounts;
	g.find_accounts(
		this->args(),
		std::inserter(accounts, accounts.begin())
	);
	align_columns(accounts, out, account::delimiter);
}

void
ggg::Edit_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
		<< this->prefix() << " [-qae] ENTITY...\n";
}




