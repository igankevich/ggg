#include <algorithm>
#include <iostream>

#include <ggg/cli/messages.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Messages::parse_arguments(int argc, char* argv[]) {
    int opt;
    ::opterr = 0;
    while ((opt = getopt(argc, argv, "hsr")) != -1) {
        switch (opt) {
            case 'h': this->_option = Option::Help; break;
            case 's': this->_option = Option::Size; break;
        }
    }
    for (int i=::optind; i<argc; ++i) {
        this->_args.emplace_back(argv[i]);
    }
}

void
ggg::Messages::execute() {
    using Statement = Database::statement_type;
    auto nargs = this->_args.size();
    Database db;
    Statement st;
    switch (this->_option) {
        case Option::Default:
            db.open(Database::File::Accounts, Database::Flag::Read_only);
            if (nargs == 0) {
                st = db.messages();
            } else if (nargs == 1) {
                const auto& name = args().front();
                st = db.messages(name.data());
            } else {
                st = db.messages(args().begin(), args().end());
            }
            for (const auto& msg : st.rows<message>()) {
                std::cout << msg << '\n';
            }
            st.close();
            break;
        case Option::Size:
            if (nargs != 0) {
                throw std::invalid_argument("please, do not specify arguments");
            }
            db.open(Database::File::Accounts, Database::Flag::Read_only);
            std::cout << db.messages_size() << '\n';
            break;
        case Option::Help:
            this->print_usage();
            break;
    }
    db.close();
}

void
ggg::Messages::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " [ENTITY...]\n";
    std::cout << "       " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " -s\n";
    std::cout << "    -s    messages disk usage in bytes\n";
}

void
ggg::Rotate_messages::parse_arguments(int argc, char* argv[]) {
    for (int i=2; i<argc; ++i) { this->_args.emplace_back(argv[i]); }
}

void
ggg::Rotate_messages::execute() {
    if (this->_args.empty()) {
        throw std::invalid_argument("please, specify at least one modifier");
    }
    Database db;
    db.open(Database::File::Accounts, Database::Flag::Read_write);
    db.rotate_messages(args().begin(), args().end());
    db.close();
}

void
ggg::Rotate_messages::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << " MODIFIER...\n"
        "You can use any valid sqlite modifier.\n"
        "Examples:\n"
        GGG_EXECUTABLE_NAME " " << this->prefix() << " '-30 days'\n"
        GGG_EXECUTABLE_NAME " " << this->prefix() << " '-1 months'\n";
}
