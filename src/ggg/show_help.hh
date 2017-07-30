#ifndef SHOW_HELP_HH
#define SHOW_HELP_HH

#include "command.hh"

namespace ggg {

	class Show_help: public Command {
	public:
		void execute(int argc, char* argv[]) override;
		void print_usage() override;
	};

}

#endif // SHOW_HELP_HH

