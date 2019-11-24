#ifndef GGG_CLI_UNLOCK_ENTITY_HH
#define GGG_CLI_UNLOCK_ENTITY_HH

#include <ggg/cli/command.hh>

namespace ggg {

	class Unlock_entity: public Command {
	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp




