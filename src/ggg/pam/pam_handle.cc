#include <algorithm>
#include <codecvt>
#include <cstring>
#include <fstream>
#include <fstream>
#include <iterator>
#include <locale>
#include <regex>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <ggg/core/entity.hh>
#include <ggg/pam/pam_handle.hh>

namespace {

    const char* key_account = "ggg_account";
    const char* key_database = "ggg_database";

    template<class T>
    const void**
    void_ptr(const T** ptr) {
        return reinterpret_cast<const void**>(ptr);
    }

    template<class T>
    const void**
    void_ptr(T** ptr) {
        return reinterpret_cast<const void**>(const_cast<const T**>(ptr));
    }

    void
    delete_database(::pam_handle_t*, void* data, int) {
        auto* db = reinterpret_cast<ggg::Database*>(data);
        if (db) { db->close(); }
        delete db;
    }

    template <class T>
    void
    delete_object(::pam_handle_t*, void* data, int) {
        delete reinterpret_cast<T*>(data);
    }

}

ggg::account*
ggg::pam_handle::get_account() {
    account* acc = nullptr;
    if (!get_data(key_account, void_ptr<account>(&acc)) || !acc) {
        throw_pam_error(errc::service_error);
    }
    return acc;
}

void
ggg::pam_handle::set_account(const account& acc) {
    set_data(key_account, new account(acc), delete_object<account>);
}

ggg::Database*
ggg::pam_handle::get_database() {
    Database* db = nullptr;
    if (!get_data(key_database, void_ptr<Database>(&db)) || !db) {
        db = new Database(Database::File::Accounts, Database::Flag::Read_only);
        set_data(key_database, db, delete_database);
    }
    return db;
}

void
ggg::pam_handle::close_connection() {
    set_data(key_database, nullptr, nullptr);
}

void
ggg::pam_handle::parse_args(int argc, const char** argv) {
    for (int i=0; i<argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "debug") {
            this->_debug = true;
        } else if (arg.find("entropy=") == 0) {
            this->_minentropy = std::strtod(arg.data() + 8, nullptr);
        } else {
            pam_syslog(*this, LOG_ERR, "unknown module argument \"%s\"", argv[i]);
        }
    }
}
