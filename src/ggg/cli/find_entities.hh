#ifndef GGG_FIND_ENTITIES_HH
#define GGG_FIND_ENTITIES_HH

#include "show_base.hh"

namespace ggg {

	class Find_entities: public Show_base {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp




