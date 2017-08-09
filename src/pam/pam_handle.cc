#include "pam_handle.hh"

#include <cstring>
#include <iterator>
#include <fstream>
#include <algorithm>
#include <vector>
#include <sstream>

#include "core/form_field.hh"
#include "config.hh"

namespace {

	const char* key_account = "ggg_account";

	int
	get_prompt(const ggg::form_field& rhs) {
		return rhs.type() == ggg::field_type::password
			? PAM_PROMPT_ECHO_OFF
			: PAM_PROMPT_ECHO_ON;
	}

	template <class Result>
	void
	read_form_fields(const ggg::account& recruiter, Result result) {
		using ggg::form_field;
		std::string path;
		path.append(GGG_REG_ROOT);
		path.push_back('/');
		path.append(recruiter.login().data());
		std::ifstream in(path);
		if (in.is_open()) {
			std::copy(
				std::istream_iterator<form_field>(in),
				std::istream_iterator<form_field>(),
				result
			);
		}
	}

	template <class Iterator>
	void
	init_messages(Iterator first, Iterator last, ggg::messages& m, bool nocolon) {
		std::for_each(
			first,
			last,
			[&m,nocolon] (const ggg::form_field& rhs) {
				if (!rhs.is_constant()) {
					std::string n = rhs.name();
					if (!nocolon) {
						n.append(": ");
					}
					m.emplace_back(get_prompt(rhs), n);
				}
			}
		);
	}

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
		} else if (arg == "nocolon") {
			this->_nocolon = true;
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
	std::vector<form_field> all_fields;
	read_form_fields(recruiter, std::back_inserter(all_fields));
	std::sort(all_fields.begin(), all_fields.end());
	messages m;
	init_messages(all_fields.begin(), all_fields.end(), m, this->_nocolon);
	responses r(m.size());
	conversation_ptr conv = this->get_conversation();
	conv->converse(m, r);
	{
		std::stringstream msg;
		msg << r;
		this->debug("responses=%s", msg.str().data());
	}
}

