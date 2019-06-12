#ifndef GGG_ADD_ENTITY_HH
#define GGG_ADD_ENTITY_HH

#include <ostream>
#include <string>

#include <unistdx/fs/path>

#include <ggg/cli/command.hh>
#include <ggg/cli/entity_type.hh>
#include <ggg/core/database.hh>
#include <ggg/guile/store.hh>

namespace ggg {

	class Add_entity: public Command {

	private:
		Entity_type _type = Entity_type::Entity;
		std::string _filename;
		std::string _expression;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;

	private:
		template <class T> void do_execute(Store& db);
		template <class T> void add_interactive(Store& db);
		template <class T> void insert(Store& db, std::string guile);

		inline Entity_type type() const { return this->_type; }

	};

}

#endif // GGG_ADD_ENTITY_HH






