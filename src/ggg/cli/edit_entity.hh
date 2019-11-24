#ifndef GGG_CLI_EDIT_ENTITY_HH
#define GGG_CLI_EDIT_ENTITY_HH

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>
#include <ggg/core/database.hh>

namespace ggg {

	class Edit_entity: public Command {

	private:
		Entity_type _type = Entity_type::Entity;
		std::string _filename;
		std::string _expression;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;

	private:

		template <class T>
		void print_objects(Database& db, std::ostream& out);

		template <class T>
		void update(Database& db, const std::string& guile);

		template <class T>
		void edit_objects(Database& db);

		template <class T>
		void edit_interactive(Database& db);

		template <class T>
		void edit_batch(Database& db);

	};

}

#endif // vim:filetype=cpp





