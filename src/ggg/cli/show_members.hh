#ifndef GGG_SHOW_MEMBERS_HH
#define GGG_SHOW_MEMBERS_HH

#include <ggg/cli/show_base.hh>
#include <ggg/core/entity.hh>

namespace ggg {

		class Show_members: public Show_base<entity> {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
	};

}

#endif // vim:filetype=cpp



