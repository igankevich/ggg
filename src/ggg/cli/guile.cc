#include <iomanip>
#include <iostream>

#include <ggg/cli/guile.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/group.hh>
#include <ggg/core/machine.hh>
#include <ggg/guile/guile_traits.hh>
#include <ggg/guile/init.hh>

void
ggg::Guile::parse_arguments(int argc, char* argv[]) {
    Command::parse_arguments(argc, argv);
    if (args().empty()) {
        print_usage();
        throw quiet_error();
    }
}

void
ggg::Guile::execute() {
    guile_init();
    for (const auto& filename : args()) {
        scm_c_primitive_load(filename.data());
    }
}

void
ggg::Guile::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " " << this->prefix() << " FILE...\n";
}
