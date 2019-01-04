#ifndef GGG_ADD_ENTITY_HH
#define GGG_ADD_ENTITY_HH

#include <ostream>
#include <string>

#include <unistdx/fs/path>

#include <ggg/core/database.hh>
#include <ggg/cli/command.hh>

namespace ggg {

	class Add_entity: public Command {

	private:
		enum struct Type {
			Directory,
			File
		};

		std::string _path;
		Type _type = Type::Directory;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;

	private:
		void generate_entities(Database& db, std::ostream& out);
		void add_entities(Database& db, sys::path filename);
		void add_interactive(Database& db);
		void add_batch(Database& db);

	};

}

#endif // GGG_ADD_ENTITY_HH






