#ifndef GGG_CLI_ATTACH_HH
#define GGG_CLI_ATTACH_HH

#include <ggg/cli/command.hh>
#include <ggg/core/ties.hh>

namespace ggg {

    struct Attach: public Command {
    protected:
        bool _group = false;
    public:
        void parse_arguments(int argc, char* argv[]) override;
        void execute() override;
        void print_usage() override;
    };

    struct Detach: public Command {
        void execute() override;
        void print_usage() override;
    };

    struct Tie: public Command {
    protected:
        Ties _tie = Ties::User_group;
    public:
        void parse_arguments(int argc, char* argv[]) override;
        void execute() override;
        void print_usage() override;
    };

    struct Untie: public Command {
        void execute() override;
        void print_usage() override;
    };

}

#endif // vim:filetype=cpp
