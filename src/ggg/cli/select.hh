#ifndef GGG_CLI_SELECT_HH
#define GGG_CLI_SELECT_HH

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

    class Select: public Command {

    private:
        Entity_type _type = Entity_type::Entity;
        Format _oformat = Format::Name;
        bool _regex = false;

    public:
        Select() = default;
        inline explicit Select(bool regex): _regex(regex) {}

        void parse_arguments(int argc, char* argv[]) override;
        void execute() override;
        void print_usage() override;

        inline Entity_type type() const { return this->_type; }
        inline Format output_format() const { return this->_oformat; }

    private:
        void select_by_name();
        void select_by_regex();

    };

}

#endif // vim:filetype=cpp
