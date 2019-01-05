#ifndef GGG_GGG_SHOW_LOCKED_HH
#define GGG_GGG_SHOW_LOCKED_HH

#include <ggg/cli/show_base.hh>
#include <ggg/core/entity.hh>

namespace ggg {

	class Show_locked: public Show_base<entity> {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
	};

}

#endif // vim:filetype=cpp





