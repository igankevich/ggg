#ifndef GGG_HEAL_HH
#define GGG_HEAL_HH

#include <ostream>
#include <string>

#include <ggg/ggg/command.hh>

namespace ggg {

	class Heal: public Command {

	public:
		void execute() override;

	};

}

#endif // vim:filetype=cpp







