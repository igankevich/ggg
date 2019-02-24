#ifndef GGG_CLI_FIND_MACHINES_HH
#define GGG_CLI_FIND_MACHINES_HH

#include <ggg/cli/show_base.hh>
#include <ggg/core/machine.hh>

namespace ggg {

	class Find_machines: public Show_base<Machine> {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp





