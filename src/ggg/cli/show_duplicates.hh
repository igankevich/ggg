#ifndef GGG_CLI_SHOW_DUPLICATES_HH
#define GGG_CLI_SHOW_DUPLICATES_HH

#include <ggg/cli/command.hh>

namespace ggg {

	class Show_duplicates: public Command {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp

