#ifndef GGG_CLI_EXPIRE_ENTITY_HH
#define GGG_CLI_EXPIRE_ENTITY_HH

#include "command.hh"

namespace ggg {

	class Expire_entity: public Command {
	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp



