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
#include "hierarchy_instance.hh"
#include "ctl/account_ctl.hh"
#include "core/entity.hh"
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
				if (rhs.is_input()) {
					std::string n = rhs.name();
					if (!nocolon) {
						n.append(": ");
					}
					m.emplace_back(get_prompt(rhs), n);
				}
			}
		);
	}

	template <class It1, class It2>
	bool
	validate_responses(It1 first, It1 last, It2 first2) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
		return std::equal(
			first, last, first2,
			[&cv] (const ggg::form_field& ff, const ggg::response& resp) {
				bool valid = true;
				if (ff.is_input()) {
					std::wregex expr(cv.from_bytes(ff.regex()));
					std::wstring field_value = cv.from_bytes(resp.text());
					valid = std::regex_match(field_value, expr);
				}
				return valid;
			}
		);
	}

	typedef std::unordered_map<ggg::form_field,const char*> field_values;

	std::string
	interpolate(std::string orig, const field_values& values, char prefix='$') {
		enum state_type {
			parsing_string,
			parsing_open_bracket,
			parsing_number,
			parsing_close_bracket
		};
		std::string result;
		std::stringstream str;
		state_type st = parsing_string;
		const char* first = orig.data();
		const char* last = orig.data() + orig.size();
		const char* start;
		while (first != last) {
			const char ch = *first;
			if (st == parsing_string) {
				if (ch == prefix) {
					st = parsing_open_bracket;
					start = first;
					str.clear();
					str.str("");
				} else {
					result.push_back(ch);
				}
			} else if (st == parsing_open_bracket) {
				if (ch == '{') {
					st = parsing_number;
				} else if (ch >= '0' && ch <= '9') {
					str.put(ch);
					st = parsing_number;
				} else {
					result.append(start, first+1);
					st = parsing_string;
				}
			} else if (st == parsing_number) {
				if (ch >= '0' && ch <= '9') {
					str.put(ch);
				} else {
					st = parsing_close_bracket;
				}
			} else if (st == parsing_close_bracket) {
				if (ch == prefix) {
					st = parsing_open_bracket;
					start = first;
					str.clear();
					str.str("");
				} else if (ch != '}') {
					result.push_back(ch);
					st = parsing_string;
				} else {
					st = parsing_string;
				}
			}
			if ((st == parsing_number && first == last-1) ||
				st == parsing_close_bracket)
			{
				bool success = false;
				ggg::form_field::id_type id = 0;
				str >> id;
				if (!str.fail()) {
					auto it = values.find(ggg::form_field(id));
					if (it != values.end()) {
						result.append(it->second);
						success = true;
					}
				}
				if (!success) {
					result.append(start, first+1);
				} else {
					if (st == parsing_close_bracket) {
						result.push_back(ch);
					}
				}
			}
			++first;
		}
		return result;
	}

	template <class It1, class It2>
	ggg::entity
	make_entity(It1 first, It1 last, It2 first2) {
		field_values values;
		ggg::entity ent;
		while (first != last) {
			if (first->type() == ggg::field_type::set) {
				if (first->target().find("entity.") == 0) {
					std::string value = interpolate(first->regex(), values);
					ent.set(*first, value.data());
				}
			} else {
				values.emplace(*first, first2->text());
			}
			++first;
			++first2;
		}
		return ent;
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
	conversation_ptr conv = this->get_conversation();
	ggg::entity ent;
	bool valid;
	do {
		responses r(m.size());
		conv->converse(m, r);
		valid = validate_responses(
			all_fields.begin(),
			all_fields.end(),
			r.begin()
		);
		if (valid) {
			ent = make_entity(
				all_fields.begin(),
				all_fields.end(),
				r.begin()
			);
		}
		std::stringstream msg;
		msg << "fields=";
		std::copy(
			all_fields.begin(),
			all_fields.end(),
			std::ostream_iterator<form_field>(msg, ",")
		);
		msg << "responses=" << r << ",ent=" << ent << ",valid=" << valid;
		this->debug("%s", msg.str().data());
	} while (!valid);
	conv->prompt("press any key...");
}

