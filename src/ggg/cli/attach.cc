#include <ggg/cli/attach.hh>

#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Attach::parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "gh")) != -1) {
        switch (opt) {
            case 'g': this->_group = true; break;
            case 'h': this->_help = true; break;
        }
    }
    for (int i=::optind; i<argc; ++i) {
        this->_args.emplace_back(argv[i]);
    }
}

void
ggg::Attach::execute()  {
    if (this->_args.size() != 2) {
        throw std::invalid_argument("please, specify exactly two names");
    }
    if (args().front() == args().back()) {
        throw std::invalid_argument("please, specify different names");
    }
    const auto& child = args().front();
    const auto& parent = args().back();
    Database db(Database::File::Entities, Database::Flag::Read_write);
    Transaction tr(db);
    db.attach(child.data(), parent.data(),
              this->_group ? Database::Ties::Group_group : Database::Ties::User_user);
    tr.commit();
}

void
ggg::Attach::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " [-gh] CHILD PARENT\n";
}


void
ggg::Detach::execute() {
    if (args().empty()) {
        throw std::invalid_argument("please, specify at least one name");
    }
    remove_duplicate_arguments();
    Database db(Database::File::Entities, Database::Flag::Read_write);
    Transaction tr(db);
    for (const auto& child : args()) { db.detach(child.data()); }
    tr.commit();
}

void
ggg::Detach::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " ENTITY...\n";
}

void
ggg::Tie::parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "t:h")) != -1) {
        switch (opt) {
            case 't': this->_tie = string_to_ties(::optarg); break;
            case 'h': this->_help = true; break;
        }
    }
    for (int i=::optind; i<argc; ++i) {
        this->_args.emplace_back(argv[i]);
    }
}

void
ggg::Tie::execute()  {
    if (this->_args.size() != 2) {
        throw std::invalid_argument("please, specify exactly two names");
    }
    if (args().front() == args().back()) {
        throw std::invalid_argument("please, specify different names");
    }
    const auto& child = args().front();
    const auto& parent = args().back();
    Database db(Database::File::Entities, Database::Flag::Read_write);
    Transaction tr(db);
    db.tie(child.data(), parent.data(), this->_tie);
    tr.commit();
}

void
ggg::Tie::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " [-t type] CHILD PARENT\n"
        "-t type    possible values: user-user, user-group, group-group\n";
}

void
ggg::Untie::execute() {
    //remove_duplicate_arguments();
    auto n = args().size();
    if (n != 1 && n != 2) {
        throw std::invalid_argument("please, specify one or two names");
    }
    const auto& child = args().front();
    Database db(Database::File::Entities, Database::Flag::Read_write);
    Transaction tr(db);
    if (n == 1) {
        db.untie(child.data());
    } else if (n == 2) {
        const auto& parent = args().back();
        db.untie(child.data(), parent.data());
    }
    tr.commit();
}

void
ggg::Untie::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " CHILD [PARENT]\n";
}
