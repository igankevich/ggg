#ifndef GGG_REMOVE_ENTITY_HH
#define GGG_REMOVE_ENTITY_HH

#include "command.hh"

namespace ggg {

	class Remove_entity: public Command {
	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // GGG_REMOVE_ENTITY_HH


