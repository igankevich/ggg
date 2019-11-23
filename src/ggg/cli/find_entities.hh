#ifndef GGG_FIND_ENTITIES_HH
#define GGG_FIND_ENTITIES_HH

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

	class Find_entities: public Command {

	private:
        Entity_type _type = Entity_type::Entity;
        Entity_output_format _oformat = Entity_output_format::TSV;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp




