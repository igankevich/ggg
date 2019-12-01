#include <algorithm>
#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <ggg/cli/cli_traits.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/cli/select_by_name.hh>
#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/database.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/message.hh>

namespace {

	template <class T, class Container>
	void
	show_objects(
        ggg::Database& db,
        const Container& args,
        ggg::Entity_output_format format
	) {
		using namespace ggg;
        auto cnt = CLI_traits<T>::select(db, args);
        write<T>(std::cout, cnt, format);
	}

}

void
ggg::Select_by_name::execute() {
	Database db;
	switch (type()) {
		case Entity_type::Entity:
			db.open(Database::File::Entities);
			show_objects<entity>(db, this->args(), this->output_format());
			break;
		case Entity_type::Account:
			db.open(Database::File::Accounts);
			show_objects<account>(db, this->args(), this->output_format());
			break;
		case Entity_type::Machine:
            throw std::runtime_error("not implemented");
			break;
		case Entity_type::Message:
			db.open(Database::File::Accounts);
			show_objects<message>(db, this->args(), this->output_format());
			break;
	}
    db.close();
}

void
ggg::Select_by_name::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " " << this->prefix()
        << " [-t TYPE] [-o FORMAT] NAME..." << '\n';
}

