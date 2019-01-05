#ifndef GGG_SHOW_BASE_HH
#define GGG_SHOW_BASE_HH

#include <set>

#include <ggg/cli/command.hh>

namespace ggg {

	/// Base class for all "show" commands.
	template <class T>
	class Show_base: public Command {

	public:
		typedef T value_type;

	protected:
		bool _long = false;
		bool _table = false;
		std::set<T> _result;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp
