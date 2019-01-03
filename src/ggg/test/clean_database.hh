#ifndef GGG_TEST_CLEAN_DATABASE_HH
#define GGG_TEST_CLEAN_DATABASE_HH

#include <cstdlib>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <sstream>
#include <unistdx/fs/mkdirs>

struct Clean_database: public ggg::Database {

	Clean_database() {
		sys::mkdirs(sys::path(GGG_ROOT));
		std::remove(GGG_DATABASE_PATH);
		this->open(GGG_DATABASE_PATH, false);
	}

	void insert(const char* str) {
		ggg::entity ent;
		std::stringstream(str) >> ent;
		this->Database::insert(ent);
	}

};

#endif // vim:filetype=cpp
