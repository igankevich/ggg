#include <iostream>

#include <ggg/cli/cli_traits.hh>
#include <ggg/cli/select_all.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>

void
ggg::Select_all::parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o': std::string(::optarg) >> this->_oformat; break;
        }
    }
    for (int i=::optind; i<argc; ++i) {
        this->_args.emplace_back(argv[i]);
    }
}

void
ggg::Select_expired::execute() {
    if (!this->_args.empty()) {
        throw std::invalid_argument("please, do not specify names");
    }
    Database db(Database::File::All);
    auto result = db.expired_entities();
    write<entity>(std::cout, result, output_format());
    result.close();
    db.close();
}

void
ggg::Select_locked::execute() {
    if (!this->_args.empty()) {
        throw std::invalid_argument("please, do not specify names");
    }
    Database db(Database::File::All);
    auto result = db.locked_entities();
    write<entity>(std::cout, result, output_format());
    result.close();
    db.close();
}

void
ggg::Select_parents::execute() {
    remove_duplicate_arguments();
    if (this->_args.empty()) {
        throw std::invalid_argument("please, specify names");
    }
    Database db(Database::File::Entities);
    for (const auto& name : this->args()) {
        auto result = db.find_parent_entities(name.data());
        write<entity>(std::cout, result, output_format());
    }
    db.close();
}

void
ggg::Select_parents::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " NAME...\n";
}

void
ggg::Select_children::execute() {
    Database db(Database::File::Entities);
    for (const auto& name : this->args()) {
        auto result = db.find_child_entities(name.data());
        write<entity>(std::cout, result, output_format());
    }
    db.close();
}

void
ggg::Select_children::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " NAME...\n";
}

void
ggg::Select_machines::execute() {
    Database db(Database::File::Entities);
    auto result = db.machines();
    write<Machine>(std::cout, result, output_format());
    result.close();
    db.close();
}
