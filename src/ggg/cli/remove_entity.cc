#include <iostream>

#include <ggg/cli/remove_entity.hh>
#include <ggg/config.hh>
#include <ggg/core/native.hh>

void
ggg::Remove_entity::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "t:")) != -1) {
		switch (opt) {
			case 't': std::string(::optarg) >> this->_type; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace_back(argv[i]);
	}
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
	remove_duplicate_arguments();
}

template <class T>
void
ggg::Remove_entity
::do_execute(Store& store) {
    Transaction tr(store);
    for (const auto& name : this->args()) { store.erase(T(name)); }
    tr.commit();
}

void
ggg::Remove_entity::execute()  {
    Store store;
    switch (this->_type) {
        case Entity_type::Entity:
            store.open(Database::File::All, Database::Flag::Read_write);
            do_execute<entity>(store);
            break;
        case Entity_type::Account:
            store.open(Database::File::Accounts, Database::Flag::Read_write);
            do_execute<account>(store);
            break;
        case Entity_type::Public_key:
            store.open(Store::File::Accounts, Store::Flag::Read_write);
            this->do_execute<public_key>(store);
            break;
        case Entity_type::Machine:
            store.open(Store::File::Entities, Store::Flag::Read_write);
            this->do_execute<public_key>(store);
            break;
        case Entity_type::Message:
            throw std::invalid_argument("not supported");
    }
}

void
ggg::Remove_entity::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " NAME...\n";
}


