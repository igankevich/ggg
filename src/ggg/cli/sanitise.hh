#ifndef GGG_GGG_SANITISE_HH
#define GGG_GGG_SANITISE_HH

#include "command.hh"

namespace ggg {

	class Sanitise: public Command {

	public:
		void execute() override;
	};

}

#endif // vim:filetype=cpp


