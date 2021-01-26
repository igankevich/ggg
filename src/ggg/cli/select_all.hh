#ifndef GGG_CLI_SELECT_ALL_HH
#define GGG_CLI_SELECT_ALL_HH

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

    class Select_all: public Command {

    protected:
        Format _oformat = Format::Name;

    public:
        void parse_arguments(int argc, char* argv[]) override;
        inline Format output_format() const { return this->_oformat; }

    };

    class Select_expired: public Select_all {
    public:
        void execute() override;
    };

    class Select_locked: public Select_all {
    public:
        void execute() override;
    };

    class Select_edges: public Select_all {
    protected:
        int _depth = 1;
    public:
        void parse_arguments(int argc, char* argv[]) override;
    };

    class Select_parents: public Select_edges {
    public:
        void execute() override;
        void print_usage() override;
    };

    class Select_children: public Select_edges {
    public:
        void execute() override;
        void print_usage() override;
    };

    class Select_groups: public Select_all {
    public:
        void execute() override;
        void print_usage() override;
    };

    class Select_members: public Select_all {
    public:
        void execute() override;
        void print_usage() override;
    };

    class Select_machines: public Select_all {
    public:
        void execute() override;
    };

}

#endif // vim:filetype=cpp
