#include "pam_handle.hh"

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

#include <ggg/config.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/form_field.hh>
#include <ggg/core/lock.hh>
#include <ggg/ctl/form.hh>
#include <ggg/ctl/ggg.hh>

namespace {

	const char* key_account = "ggg_account";

	template<class T>
	static const void**
	void_ptr(const T** ptr) {
		return reinterpret_cast<const void**>(ptr);
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
		} else if (arg == "register") {
			this->_allowregister = true;
		} else if (arg.find("type=") == 0) {
			this->_type = from_string(arg.data() + 5);
		} else if (arg.find("entropy=") == 0) {
			this->_minentropy = std::strtod(arg.data() + 8, nullptr);
		} else if (arg.find("rounds=") == 0) {
			const int tmp = std::atoi(arg.data() + 7);
			if (tmp > 0) {
				this->_nrounds = tmp;
			} else {
				pam_syslog(
					*this,
					LOG_ERR,
					"bad module argument \"%s\"",
					argv[i]
				);
			}
		} else {
			pam_syslog(
				*this,
				LOG_ERR,
				"unknown module argument \"%s\"",
				argv[i]
			);
		}
	}
}
