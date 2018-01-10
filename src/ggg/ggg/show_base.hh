#ifndef GGG_SHOW_BASE_HH
#define GGG_SHOW_BASE_HH

#include <set>

#include <ggg/core/entity.hh>

#include "command.hh"

namespace ggg {

	/// Base class for all "show" commands.
	class Show_base: public Command {

	protected:
		bool _long = false;
		bool _table = false;
		std::set<wentity> _result;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp
