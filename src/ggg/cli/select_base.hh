#ifndef GGG_CLI_SELECT_BASE_HH
#define GGG_CLI_SELECT_BASE_HH

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

	class Select_base: public Command {

	protected:
        Entity_type _type = Entity_type::Entity;
        Format _oformat = Format::Name;

	public:
		void parse_arguments(int argc, char* argv[]) override;

        inline Entity_type type() const { return this->_type; }
        inline Format output_format() const { return this->_oformat; }

	};

}

#endif // vim:filetype=cpp

