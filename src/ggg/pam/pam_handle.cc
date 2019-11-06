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
	const char* key_message = "ggg_message";

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

    void
    delete_log_message(::pam_handle_t*, void* data, int) {
        delete reinterpret_cast<ggg::Database*>(data);
    }

}

void
ggg::delete_account(pam_handle_t *pamh, void *data, int error_status) {
	account* acc = reinterpret_cast<account*>(data);
	delete acc;
}


const char*
ggg::pam_handle::get_user() const {
	const char* user = nullptr;
	if (pam_get_user(*this, &user, nullptr) != PAM_SUCCESS) {
		throw_pam_error(pam_errc::authentication_error);
	}
	return user;
}

const char*
ggg::pam_handle::get_password(pam_errc err) const {
	const char* pwd = nullptr;
	if (pam_get_authtok(
		*this,
		PAM_AUTHTOK,
		&pwd,
		nullptr
	) != PAM_SUCCESS) {
		throw_pam_error(err);
	}
	return pwd;
}

const char*
ggg::pam_handle::get_old_password() const {
	const char* pwd = nullptr;
	if (pam_get_authtok(
		*this,
		PAM_OLDAUTHTOK,
		&pwd,
		nullptr
	) != PAM_SUCCESS) {
		throw_pam_error(pam_errc::authtok_recovery_error);
	}
	return pwd;
}

const ggg::account*
ggg::pam_handle::get_account() const {
	const account* acc = nullptr;
	if (pam_get_data(
		*this,
		key_account,
		void_ptr<account>(&acc)
	) != PAM_SUCCESS || !acc) {
		throw_pam_error(pam_errc::service_error);
	}
	return acc;
}

ggg::Database&
ggg::pam_handle::get_database() {
	Database* db = nullptr;
	if (!get_data(key_database, void_ptr<Database>(&db)) || !db) {
        db = new Database(Database::File::Accounts, Database::Flag::Read_write);
        set_data(key_database, db, delete_database);
	}
	return *db;
}

const ggg::log_message&
ggg::pam_handle::get_message() {
	log_message* msg = nullptr;
	if (!get_data(key_message, void_ptr<log_message>(&msg)) || !msg) {
        msg = new log_message;
        set_data(key_message, msg, delete_log_message);
	}
	return *msg;
}

void
ggg::pam_handle::set_account(const ggg::account& acc) {
	if (pam_set_data(
		*this,
		key_account,
		new ggg::account(acc),
		delete_account
	) != PAM_SUCCESS) {
		throw_pam_error(pam_errc::service_error);
	}
}

void
ggg::pam_handle::set_item(int key, const void* value) {
	pam::call(::pam_set_item(*this, key, value));
}

ggg::conversation_ptr
ggg::pam_handle::get_conversation() const {
	conversation* ptr = nullptr;
	int ret = ::pam_get_item(*this, PAM_CONV, (const void**)&ptr);
	if (ret != PAM_SUCCESS) {
		throw_pam_error(pam_errc(ret));
	}
	if (!ptr || !ptr->conv) {
		throw_pam_error(pam_errc::conversation_error);
	}
	return conversation_ptr(ptr);
}

ggg::pam_errc
ggg::pam_handle::handle_error(const std::system_error& e, pam_errc def) const {
	pam_errc ret;
	if (e.code().category() == pam_category) {
		ret = pam_errc(e.code().value());
		print_error(e);
	} else if (e.code().category() == std::iostream_category()) {
		ret = def;
		pam_syslog(*this, LOG_CRIT, "%s", e.what());
	} else {
		ret = def;
		print_error(e);
	}
	return ret;
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
