#ifndef GGG_SHOW_MEMBERS_HH
#define GGG_SHOW_MEMBERS_HH

#include "command.hh"

namespace ggg {

	class Show_members: public Command {

	private:
		bool _long = false;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp



