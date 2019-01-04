#ifndef GGG_EDIT_ENTITY_HH
#define GGG_EDIT_ENTITY_HH

#include <ggg/core/database.hh>
#include <ggg/ggg/command.hh>

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
		void print_objects(Database& db, std::ostream& out);

		template <class T>
		void update_objects(Database& db, const std::string& filename);

		template <class T>
		void edit_objects(Database& db);

		template <class T>
		void edit_interactive(Database& db);

		template <class T>
		void edit_batch(Database& db);

		void edit_directory();

	};

}

#endif // GGG_EDIT_ENTITY_HH





