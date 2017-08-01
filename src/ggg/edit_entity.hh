#ifndef GGG_EDIT_ENTITY_HH
#define GGG_EDIT_ENTITY_HH

#include "command.hh"
#include "ggg.hh"

namespace ggg {

	class Edit_entity: public Command {

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

	private:
		void print_entities(Ggg& g, std::ostream& out);
		void print_accounts(Ggg& g, std::ostream& out);
	};

}

#endif // GGG_EDIT_ENTITY_HH





