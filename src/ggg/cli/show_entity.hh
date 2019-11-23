#ifndef GGG_SHOW_ENTITY_HH
#define GGG_SHOW_ENTITY_HH

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

	class Show_entity: public Command {

	private:
        Entity_type _type = Entity_type::Entity;
        Entity_output_format _oformat = Entity_output_format::Rec;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


