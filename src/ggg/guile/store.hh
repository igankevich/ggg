#ifndef GGG_CLI_STORE_HH
#define GGG_CLI_STORE_HH

#include <ggg/core/database.hh>
#include <ggg/cli/entity_type.hh>

namespace ggg {

	class Store: public Database {

	public:
		using Database::Database;

		bool has(const entity& rhs);
		bool has(const account& rhs);
		bool has(const form2& rhs);

		void add(const entity& rhs);
		void add(const account& rhs);
		void add(const form2& rhs);

	};

	void store_define_procedures();

}

#endif // vim:filetype=cpp
