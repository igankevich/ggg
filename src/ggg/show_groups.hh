#ifndef GGG_SHOW_GROUPS_HH
#define GGG_SHOW_GROUPS_HH

#include "command.hh"

namespace ggg {

	class Show_groups: public Command {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp




