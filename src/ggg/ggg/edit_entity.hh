#ifndef GGG_EDIT_ENTITY_HH
#define GGG_EDIT_ENTITY_HH

#include "command.hh"
#include <ggg/ctl/ggg.hh>

namespace ggg {

	class Edit_entity: public Command {

	public:
		enum struct Type {
			Account,
			Entity,
			Directory
		};

	private:
		Type _type = Type::Entity;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;

	private:

		template <class T>
		void print_objects(GGG& g, std::ostream& out);

		template <class T>
		void update_objects(GGG& g, const std::string& filename);

		template <class T>
		void edit_objects(GGG& g);

		void edit_directory();

	};

}

#endif // GGG_EDIT_ENTITY_HH




