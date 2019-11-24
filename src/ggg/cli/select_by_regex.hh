#ifndef GGG_CLI_SELECT_BY_REGEX_HH
#define GGG_CLI_SELECT_BY_REGEX_HH

#include <ggg/cli/entity_type.hh>
#include <ggg/cli/select_base.hh>

namespace ggg {

	class Select_by_regex: public Select_base {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp




