#ifndef GGG_CLI_DOT_HH
#define GGG_CLI_DOT_HH

#include <ggg/cli/command.hh>

namespace ggg {

	class Dot: public Command {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


