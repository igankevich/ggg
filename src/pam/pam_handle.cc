#include "pam_handle.hh"

#include <cstring>
#include <iterator>
#include <fstream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <regex>
#include <locale>
#include <codecvt>
#include <tuple>
#include <unordered_map>
#include <fstream>

#include "core/form_field.hh"
#include "ctl/ggg.hh"
#include "ctl/form.hh"
#include "core/entity.hh"
#include "core/native.hh"
#include <core/lock.hh>
#include <config.hh>

namespace {

	const char* key_account = "ggg_account";

}

void
ggg::delete_account(pam_handle_t *pamh, void *data, int error_status) {
	account* acc = reinterpret_cast<account*>(data);
	delete acc;
	data = nullptr;
}


const char*
ggg::pam_handle::get_user() const {
	const char* user = nullptr;
	if (pam_get_user(*this, &user, nullptr) != PAM_SUCCESS) {
		throw_pam_error(pam_errc::auth_error);
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
		void_ptr(&acc)
	) != PAM_SUCCESS || !acc) {
		throw_pam_error(pam_errc::auth_error);
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
		throw_pam_error(pam_errc::auth_error);
	}
}

ggg::conversation_ptr
ggg::pam_handle::get_conversation() const {
	conversation* ptr = nullptr;
	int ret = ::pam_get_item(*this, PAM_CONV, (const void**)&ptr);
	if (ret != PAM_SUCCESS) {
		throw_pam_error(pam_errc(ret));
	}
	if (!ptr && !ptr->conv) {
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
		pam_syslog(*this, LOG_CRIT, e.what());
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

void
ggg::pam_handle::register_new_user(const account& recruiter) {
	form f(recruiter);
	f.set_type(this->_type);
	bool success, stopped = false;
	do {
		try {
			entity ent;
			account acc;
			std::tie(ent, acc) = f.input_entity(this);
			GGG g(GGG_ENT_ROOT, this->debug());
			bool has_uid = ent.has_id();
			bool has_gid = ent.has_gid();
			if (!has_uid || !has_gid) {
				sys::uid_type id = g.next_uid();
				if (!has_uid) {
					ent.set_uid(id);
				}
				if (!has_gid) {
					ent.set_gid(id);
				}
			}
			{
				file_lock lock(true);
				g.add(ent, ent.origin(), acc);
			}
			success = true;
		} catch (const std::system_error& err) {
			success = false;
			if (pam_errc(err.code().value()) == pam_errc::conversation_error) {
				stopped = true;
			} else {
				this->get_conversation()->error(err.what());
			}
		} catch (const std::exception& err) {
			success = false;
			this->get_conversation()->error(err.what());
		}
	} while (!success && !stopped);
	if (success) {
		this->get_conversation()->info(native("Registered successfully!").data());
	}
	if (this->_type == form_type::console) {
		this->get_conversation()->prompt("press any key to continue...");
	}
}

