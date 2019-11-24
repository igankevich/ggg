#ifndef GGG_CLI_REMOVE_ENTITY_HH
#define GGG_CLI_REMOVE_ENTITY_HH

#include "command.hh"

namespace ggg {

	class Remove_entity: public Command {
	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


