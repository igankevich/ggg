#ifndef GGG_ACTIVATE_ENTITY_HH
#define GGG_ACTIVATE_ENTITY_HH

#include "command.hh"

namespace ggg {

	class Restore_entity: public Command {
	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // GGG_ACTIVATE_ENTITY_HH




