#include "command.hh"

#include <algorithm>
#include <iostream>
#include <unistd.h>

#include <ggg/config.hh>

void
ggg::Command::parse_arguments(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "qh")) != -1) {
        if (opt == 'q') {
            this->_verbose = false;
        }
        if (opt == 'h') {
            print_usage();
            std::exit(EXIT_SUCCESS);
        }
        if (opt == '?') {
            print_usage();
            std::exit(EXIT_FAILURE);
        }
    }
    for (int i=::optind; i<argc; ++i) {
        this->_args.emplace_back(argv[i]);
    }
}

void
ggg::Command::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
        << this->prefix() << '\n';
}

void
ggg::Command::remove_duplicate_arguments() {
    std::sort(this->_args.begin(), this->_args.end());
    auto end = std::unique(this->_args.begin(),  this->_args.end());
    this->_args.resize(std::distance(this->_args.begin(), end));
}
