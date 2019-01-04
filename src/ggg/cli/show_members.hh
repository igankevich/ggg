#ifndef GGG_SHOW_MEMBERS_HH
#define GGG_SHOW_MEMBERS_HH

#include "show_base.hh"

namespace ggg {

	class Show_members: public Show_base {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
	};

}

#endif // vim:filetype=cpp



