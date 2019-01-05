#ifndef GGG_FIND_ENTITIES_HH
#define GGG_FIND_ENTITIES_HH

#include <ggg/cli/show_base.hh>
#include <ggg/core/entity.hh>

namespace ggg {

	class Find_entities: public Show_base<entity> {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp




