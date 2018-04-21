#ifndef GGG_GGG_TEST_LOCK_HH
#define GGG_GGG_TEST_LOCK_HH

#include "command.hh"

namespace ggg {

	class Test_lock: public Command {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


