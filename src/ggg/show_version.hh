#ifndef SHOW_VERSION_HH
#define SHOW_VERSION_HH

#include "command.hh"

namespace ggg {

	class Show_version: public Command {
	public:
		void execute() override;
	};

}

#endif // SHOW_VERSION_HH
