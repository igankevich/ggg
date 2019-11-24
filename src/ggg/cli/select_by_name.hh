#ifndef GGG_CLI_SELECT_BY_NAME_HH
#define GGG_CLI_SELECT_BY_NAME_HH

#include <ggg/cli/entity_type.hh>
#include <ggg/cli/select_base.hh>

namespace ggg {

	class Select_by_name: public Select_base {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


