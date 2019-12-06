#ifndef GGG_CLI_REMOVE_ENTITY_HH
#define GGG_CLI_REMOVE_ENTITY_HH

#include <ggg/cli/command.hh>
#include <ggg/guile/store.hh>

namespace ggg {

	class Remove_entity: public Command {

	private:
		Entity_type _type = Entity_type::Entity;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;

    private:
		template <class T> void do_execute(Store& db);

		inline Entity_type type() const { return this->_type; }

	};

}

#endif // vim:filetype=cpp


