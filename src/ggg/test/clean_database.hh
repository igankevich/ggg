#ifndef GGG_TEST_CLEAN_DATABASE_HH
#define GGG_TEST_CLEAN_DATABASE_HH

#include <sstream>
#include <cstdlib>

#include <ggg/config.hh>
#include <ggg/core/database.hh>

struct Clean_database: public ggg::Database {

    using ggg::Database::insert;

    Clean_database() {
        try {
            { Database(Database::File::Entities, Database::Flag::Read_write); }
            { Database(Database::File::Accounts, Database::Flag::Read_write); }
        } catch (...) {
            std::remove(GGG_ENTITIES_PATH);
            std::remove(GGG_ENTITIES_PATH "-shm");
            std::remove(GGG_ENTITIES_PATH "-wal");
            std::remove(GGG_ACCOUNTS_PATH);
            std::remove(GGG_ACCOUNTS_PATH "-shm");
            std::remove(GGG_ACCOUNTS_PATH "-wal");
            { Database(Database::File::Entities, Database::Flag::Read_write); }
            { Database(Database::File::Accounts, Database::Flag::Read_write); }
        }
        this->open(ggg::Database::File::All, ggg::Database::Flag::Read_write);
        this->db()->execute(
                "DELETE FROM entities;"
                "DELETE FROM ties;"
                "DELETE FROM hierarchy;"
                "DELETE FROM accounts;"
                "DELETE FROM messages;"
                "DELETE FROM public_keys;"
                "DELETE FROM hosts;"
                "DELETE FROM networks;"
                "DELETE FROM addresses;"
                "PRAGMA journal_mode=OFF;");
    }

    void insert(const char* str) {
        ggg::entity ent;
        std::stringstream(str) >> ent;
        this->Database::insert(ent);
    }

};

#endif // vim:filetype=cpp
