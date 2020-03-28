#include <iostream>
#include <string>
#include <vector>

#include <ggg/cli/cli_traits.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/cli/search.hh>
#include <ggg/cli/select.hh>
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
        ggg::Format format
    ) {
        using namespace ggg;
        auto st = CLI_traits<T>::select(db, args);
        write<T>(std::cout, st, format);
        st.close();
    }

}

void
ggg::Select::parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "t:o:s")) != -1) {
        switch (opt) {
            case 't': std::string(::optarg) >> this->_type; break;
            case 'o': std::string(::optarg) >> this->_oformat; break;
            case 's': this->_regex = true;
        }
    }
    for (int i=::optind; i<argc; ++i) {
        this->_args.emplace_back(argv[i]);
    }
}

void
ggg::Select::execute() {
    if (this->_regex) { select_by_regex(); }
    else { select_by_name(); }
}

void
ggg::Select::select_by_name() {
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
        case Entity_type::Public_key:
            db.open(Database::File::Accounts);
            show_objects<public_key>(db, this->args(), this->output_format());
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
ggg::Select::select_by_regex() {
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
        case Entity_type::Public_key:
            db.open(Database::File::Accounts);
            search.open(&db, this->args_begin(), this->args_end());
            st = db.search_public_keys();
            write<public_key>(std::cout, st, this->output_format());
            break;
        case Entity_type::Machine:
            db.open(Database::File::Entities);
            search.open(&db, this->args_begin(), this->args_end());
            st = db.search_machines();
            write<Machine>(std::cout, st, this->output_format());
            break;
        case Entity_type::Message:
            db.open(Database::File::Accounts);
            search.open(&db, this->args_begin(), this->args_end());
            st = db.search_messages();
            write<message>(std::cout, st, this->output_format());
            break;
    }
}

void
ggg::Select::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " " << this->prefix()
        << " [-t TYPE] [-o FORMAT] [-s] NAME..." << '\n';
}
