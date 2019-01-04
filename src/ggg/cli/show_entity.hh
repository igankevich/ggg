#ifndef GGG_SHOW_ENTITY_HH
#define GGG_SHOW_ENTITY_HH

#include "command.hh"

namespace ggg {

	class Show_entity: public Command {

	public:
		enum struct Type {
			Account,
			Entity
		};

	private:
		Type _type = Type::Entity;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


