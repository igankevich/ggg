#ifndef GGG_SHOW_DUPLICATES_HH
#define GGG_SHOW_DUPLICATES_HH

#include "command.hh"

namespace ggg {

	class Show_duplicates: public Command {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp

