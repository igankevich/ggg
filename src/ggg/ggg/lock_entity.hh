#ifndef GGG_GGG_LOCK_ENTITY_HH
#define GGG_GGG_LOCK_ENTITY_HH

#include <ggg/ggg/command.hh>

namespace ggg {

	class Lock_entity: public Command {
	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp




