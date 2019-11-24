#ifndef GGG_CLI_SHOW_HELP_HH
#define GGG_CLI_SHOW_HELP_HH

#include "command.hh"
#include "all_commands.hh"

namespace ggg {

	class Show_help: public Command {

	private:
		command_ptr _ptr;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp

