#ifndef GGG_HEAL_HH
#define GGG_HEAL_HH

#include <ostream>
#include <string>

#include "command.hh"
#include "ctl/ggg.hh"

namespace ggg {

	class Heal: public Command {

	public:
		void execute() override;

	};

}

#endif // vim:filetype=cpp







