#ifndef GGG_SHOW_GROUPS_HH
#define GGG_SHOW_GROUPS_HH

#include <ggg/cli/show_base.hh>
#include <ggg/core/entity.hh>

namespace ggg {

	class Show_groups: public Show_base<entity> {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
	};

}

#endif // vim:filetype=cpp




