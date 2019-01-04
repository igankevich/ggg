#ifndef GGG_GGG_COPY_HH
#define GGG_GGG_COPY_HH

#include <ggg/cli/command.hh>

namespace ggg {

	class Copy: public Command {

	public:
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


