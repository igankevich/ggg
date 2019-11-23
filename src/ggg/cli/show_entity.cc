#include <algorithm>
#include <chrono>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <unistdx/it/intersperse_iterator>

#include <ggg/cli/cli_traits.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/cli/show_entity.hh>
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
        /* TODO this is not efficient anymore
		if (!args.empty() &&
            cnt.size() != args.size() &&
            !std::is_same<T,message>::value) {
            std::vector<std::string> names, not_found;
            names.reserve(cnt.size());
            for (const auto& obj : cnt) { names.emplace_back(obj.name()); }
            std::sort(names.begin(), names.end());
            auto new_args = args;
            std::sort(new_args.begin(), new_args.end());
            std::set_difference(
                new_args.begin(), new_args.end(),
                names.begin(), names.end(),
                std::back_inserter(not_found)
            );
            for (const auto& name : not_found) {
                std::clog << name << " not found\n";
            }
            throw quiet_error();
		}*/
        write<T>(std::cout, cnt, format);
	}

}

void
ggg::Show_entity::parse_arguments(int argc, char* argv[]) {
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
ggg::Show_entity::execute() {
	Database db;
	switch (this->_type) {
		case Entity_type::Entity:
			db.open(Database::File::Entities);
			show_objects<entity>(db, this->args(), this->_oformat);
			break;
		case Entity_type::Account:
			db.open(Database::File::Accounts);
			show_objects<account>(db, this->args(), this->_oformat);
			break;
		case Entity_type::Machine:
            throw std::runtime_error("not implemented");
			break;
		case Entity_type::Message:
			db.open(Database::File::Accounts);
			show_objects<message>(db, this->args(), this->_oformat);
			break;
	}
    db.close();
}

void
ggg::Show_entity::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-t TYPE] [-o FORMAT] NAME..." << '\n';
}

