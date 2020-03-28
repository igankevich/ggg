#ifndef GGG_CLI_GUILE_HH
#define GGG_CLI_GUILE_HH

#include <ggg/cli/command.hh>

namespace ggg {

    class Guile: public Command {

    public:
        void parse_arguments(int argc, char* argv[]) override;
        void execute() override;
        void print_usage() override;

    };

}

#endif // vim:filetype=cpp
