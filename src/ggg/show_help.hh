#ifndef SHOW_HELP_HH
#define SHOW_HELP_HH

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

#endif // SHOW_HELP_HH

