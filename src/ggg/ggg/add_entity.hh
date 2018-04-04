#ifndef GGG_ADD_ENTITY_HH
#define GGG_ADD_ENTITY_HH

#include <ostream>
#include <string>

#include <unistdx/fs/path>

#include "command.hh"
#include <ggg/ctl/ggg.hh>

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
		void generate_entities(GGG& g, std::ostream& out);
		void add_entities(GGG& g, sys::path filename);
		void add_interactive(GGG& g);
		void add_batch(GGG& g);
		sys::path get_filename(const entity& ent) const;

	};

}

#endif // GGG_ADD_ENTITY_HH






