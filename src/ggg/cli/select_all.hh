#ifndef GGG_CLI_SELECT_ALL_HH
#define GGG_CLI_SELECT_ALL_HH

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

    class Select_all: public Command {

	protected:
        Entity_output_format _oformat = Entity_output_format::Name;

	public:
		void parse_arguments(int argc, char* argv[]) override;
        inline Entity_output_format output_format() const { return this->_oformat; }

    };

	class Select_expired: public Select_all {
	public:
		void execute() override;
	};

	class Select_locked: public Select_all {
	public:
		void execute() override;
	};

	class Select_parents: public Select_all {
	public:
		void execute() override;
		void print_usage() override;
	};

	class Select_children: public Select_all {
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




