#ifndef GGG_GGG_SHOW_LOCKED_HH
#define GGG_GGG_SHOW_LOCKED_HH

#include <ggg/cli/show_base.hh>

namespace ggg {

	class Show_locked: public Show_base {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
	};

}

#endif // vim:filetype=cpp





