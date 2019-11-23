#ifndef GGG_SHOW_BASE_HH
#define GGG_SHOW_BASE_HH

#include <vector>

#include <sqlitex/statement.hh>

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

	/// Base class for all "show" commands.
	template <class T>
	class Show_base: public Command {

	public:
		typedef T value_type;

	protected:
        Entity_type _type = Entity_type::Entity;
        Entity_output_format _oformat = Entity_output_format::Name;
        sqlite::statement _result;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
	};

}

#endif // vim:filetype=cpp
