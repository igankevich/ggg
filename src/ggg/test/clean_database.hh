#ifndef GGG_TEST_CLEAN_DATABASE_HH
#define GGG_TEST_CLEAN_DATABASE_HH

#include <cstdlib>
#include <sstream>

#include <unistdx/fs/mkdirs>

#include <ggg/config.hh>
#include <ggg/core/database.hh>

struct Clean_database: public ggg::Database {

    using ggg::Database::insert;

	Clean_database() {
		sys::mkdirs(sys::path(GGG_ROOT));
		std::remove(GGG_ENTITIES_PATH);
		std::remove(GGG_ACCOUNTS_PATH);
		{ Database(Database::File::Entities, Database::Flag::Read_write); }
		{ Database(Database::File::Accounts, Database::Flag::Read_write); }
		this->open(ggg::Database::File::All, ggg::Database::Flag::Read_write);
	}

	void insert(const char* str) {
		ggg::entity ent;
		std::stringstream(str) >> ent;
		this->Database::insert(ent);
	}

};

#endif // vim:filetype=cpp
