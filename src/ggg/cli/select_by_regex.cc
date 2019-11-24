#include <iostream>

#include <ggg/cli/cli_traits.hh>
#include <ggg/cli/select_by_regex.hh>
#include <ggg/cli/search.hh>
#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/core/entity.hh>

void
ggg::Select_by_regex::execute() {
	Database db;
    Search search;
    Database::statement_type st;
	switch (type()) {
		case Entity_type::Entity:
			db.open(Database::File::Entities);
            search.open(&db, this->args_begin(), this->args_end());
            st = db.search_entities();
            write<entity>(std::cout, st, this->output_format());
			break;
		case Entity_type::Account:
			db.open(Database::File::Accounts);
            search.open(&db, this->args_begin(), this->args_end());
            st = db.search_accounts();
            write<account>(std::cout, st, this->output_format());
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
ggg::Select_by_regex::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " " << this->prefix()
        << " [-t TYPE] [-o FORMAT] REGEX..." << '\n';
}
