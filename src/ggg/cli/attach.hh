#ifndef GGG_CLI_ATTACH_HH
#define GGG_CLI_ATTACH_HH

#include <ggg/cli/command.hh>

namespace ggg {

    struct Attach: public Command {
        void execute() override;
        void print_usage() override;
    };

    struct Detach: public Command {
        void execute() override;
        void print_usage() override;
    };

    struct Tie: public Command {
        void execute() override;
        void print_usage() override;
    };

    struct Untie: public Command {
        void execute() override;
        void print_usage() override;
    };

}

#endif // vim:filetype=cpp
