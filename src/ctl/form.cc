#include "form.hh"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iterator>
#include <locale>
#include <regex>
#include <sstream>
#include <string>

#include <config.hh>
#include <pam/conversation.hh>
#include <ctl/password.hh>

namespace {

	inline int
	get_prompt(const ggg::form_field& rhs) {
		return rhs.type() == ggg::field_type::password
			? PAM_PROMPT_ECHO_OFF
			: PAM_PROMPT_ECHO_ON;
	}

	template <class Container>
	void
	init_messages(const Container& cnt, ggg::messages& m, bool add_colon) {
		std::for_each(
			cnt.begin(), cnt.end(),
			[&m,add_colon] (const ggg::form_field& rhs) {
				if (rhs.is_input()) {
					std::string n = rhs.name();
					if (add_colon) {
						n.append(": ");
					}
					m.emplace_back(get_prompt(rhs), n);
				}
			}
		);
	}

}

void
ggg::form::read_fields(const account& recruiter) {
	std::string path;
	path.append(GGG_REG_ROOT);
	path.push_back('/');
	path.append(recruiter.login().data());
	std::ifstream in(path);
	in.imbue(std::locale::classic());
	if (in.is_open()) {
		std::copy(
			std::istream_iterator<form_field>(in),
			std::istream_iterator<form_field>(),
			std::back_inserter(this->_fields)
		);
		std::sort(this->_fields.begin(), this->_fields.end());
	}
}

std::tuple<ggg::entity,ggg::account>
ggg::form::input_entity(ggg::pam_handle* pamh) {
	messages m;
	init_messages(this->_fields, m, this->is_console());
	conversation_ptr conv = pamh->get_conversation();
	entity ent;
	account acc;
	bool valid;
	do {
		responses r(m.size());
		conv->converse(m, r);
		if (!r.ok()) {
			throw std::invalid_argument("null response");
		}
		if (r.size() != m.size()) {
			throw std::invalid_argument("bad response");
		}
		valid = this->validate(r);
		if (valid) {
			std::tie(ent, acc) = make_entity_and_account(r, pamh);
		}
		std::stringstream msg;
		msg << "fields=";
		std::copy(
			this->_fields.begin(),
			this->_fields.end(),
			std::ostream_iterator<form_field>(msg, ",")
		);
		msg << "ent=" << ent
			<< ",acc=" << acc
			<< ",valid=" << valid;
		pamh->debug("%s", msg.str().data());
	} while (!valid);
	return std::make_tuple(ent, acc);
}

bool
ggg::form::validate(const responses& r) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	return std::equal(
		this->_fields.begin(), this->_fields.end(), r.begin(),
		[&cv] (const form_field& ff, const response& resp) {
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

std::tuple<ggg::entity,ggg::account>
ggg::form::make_entity_and_account(const responses& r, ggg::pam_handle* pamh) {
	field_values values;
	entity ent;
	account acc;
	auto first = this->_fields.begin();
	auto last = this->_fields.end();
	auto first2 = r.begin();
	while (first != last) {
		if (first->type() == field_type::set) {
			bool is_entity_field = first->target().find("entity.") == 0;
			bool is_account_field = first->target().find("account.") == 0;
			if (is_entity_field || is_account_field) {
				std::string value = interpolate(first->regex(), values);
				if (is_account_field) {
					acc.set(*first, value.data());
				} else {
					ent.set(*first, value.data());
				}
			}
		} else if (first->type() == field_type::set_secure) {
			if (first->target() == "account.password") {
				form_field::id_type id = 0;
				std::stringstream str(first->regex());
				char ch = str.get();
				if (ch != '$') {
					str.putback(ch);
				}
				str >> id;
				auto it = values.find(form_field(id));
				if (it != values.end()) {
					const char* new_password = it->second;
					ggg::secure_string encrypted = ggg::encrypt(
						new_password,
						ggg::account::password_prefix(
							ggg::generate_salt(),
							pamh->password_id(),
							pamh->num_rounds()
						)
					);
					acc.set_password(encrypted);
				}
			}
		} else {
			values.emplace(*first, first2->text());
		}
		++first;
		++first2;
	}
	return std::make_tuple(ent, acc);
}

std::string
ggg::interpolate(std::string orig, const field_values& values, char prefix) {
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
			form_field::id_type id = 0;
			str >> id;
			if (!str.fail()) {
				auto it = values.find(form_field(id));
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

