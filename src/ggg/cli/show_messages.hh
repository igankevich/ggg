#ifndef GGG_CLI_SHOW_MESSAGES_HH
#define GGG_CLI_SHOW_MESSAGES_HH

#include <ggg/cli/command.hh>

namespace ggg {

	class Show_messages: public Command {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp

