#include <ggg/cli/dot.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <iostream>

void
ggg::Dot::execute() {
    Database db(Database::File::Entities, Database::Flag::Read_only);
    db.dot(std::cout);
}

void
ggg::Dot::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " " << this->prefix() << '\n';
}
