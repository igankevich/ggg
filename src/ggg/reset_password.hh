#ifndef GGG_RESET_PASSWORD_HH
#define GGG_RESET_PASSWORD_HH

#include "command.hh"

namespace ggg {

	class Reset_password: public Command {
	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp



