#include "form.hh"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <regex>
#include <sstream>
#include <string>

#include <ggg/config.hh>
#include <ggg/ctl/password.hh>
#include <ggg/sec/echo_guard.hh>
#ifndef GGG_DISABLE_PAM
#include <ggg/pam/conversation.hh>
#endif

namespace {

	#ifndef GGG_DISABLE_PAM
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
			cnt.begin(),
			cnt.end(),
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

	#endif

	template <class Iterator, class Iterator2>
	std::tuple<ggg::entity,ggg::account>
	make_entity_and_account_2(
		Iterator first,
		Iterator last,
		Iterator2 first2,
		std::string password_id,
		unsigned int num_rounds,
		double min_entropy
	) {
		using namespace ggg;
		field_values values;
		entity ent;
		account acc;
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
						ggg::validate_password(new_password, min_entropy);
						ggg::secure_string encrypted =
							ggg::encrypt(
								new_password,
								ggg::account::password_prefix(
									ggg::generate_salt(),
									password_id,
									num_rounds
								)
							);
						acc.set_password(encrypted);
					}
				}
			} else {
				values.emplace(*first, first2->data());
			}
			++first;
			++first2;
		}
		return std::make_tuple(ent, acc);
	}

}

void
ggg::form
::open(const char* name) {
	std::string path;
	path.append(GGG_REG_ROOT);
	path.push_back('/');
	path.append(name);
	std::ifstream in(path);
	if (!in.is_open()) {
		throw std::invalid_argument("unknown form");
	}
	in.imbue(std::locale::classic());
	std::copy(
		std::istream_iterator<form_field>(in),
		std::istream_iterator<form_field>(),
		std::back_inserter(this->_fields)
	);
	std::sort(this->_fields.begin(), this->_fields.end());
	for (const form_field& ff : this->_fields) {
		if (ff.type() == field_type::set && ff.target() == "entropy") {
			try {
				this->_minentropy = std::max(0.0, std::stod(ff.regex()));
			} catch (const std::exception& err) {
				std::cerr << std::endl << "bad entropy" << std::endl;
				throw;
			}
		}
	}
}

std::tuple<ggg::entity,ggg::account>
ggg::form
::input_entity() {
	entity ent;
	account acc;
	std::vector<std::string> responses;
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	for (const form_field& ff : this->_fields) {
		if (ff.is_input()) {
			bool valid = false;
			std::wstring name = cv.from_bytes(ff.name());
			std::wregex expr(cv.from_bytes(ff.regex()));
			std::wstring value;
			const int max_iterations = 100;
			int i = 0;
			do {
				value.clear();
				std::wcout << name << ": " << std::flush;
				if (ff.type() == field_type::password) {
					echo_guard g(STDIN_FILENO);
					std::getline(std::wcin, value, L'\n');
				} else {
					std::getline(std::wcin, value, L'\n');
				}
				if (std::wcin) {
					if (ff.type() == field_type::password) {
						std::string p = cv.to_bytes(value);
						try {
							ggg::validate_password(p.data(), this->_minentropy);
							valid = std::regex_match(value, expr);
						} catch (const std::exception& err) {
							std::cerr << std::endl << err.what() << std::endl;
							valid = false;
						}
					} else {
						valid = std::regex_match(value, expr);
					}
				} else {
					std::wcin.clear();
				}
			} while (!valid && ++i < max_iterations);
			if (!valid) {
				throw std::runtime_error("reached maximum number of attempts");
			}
			responses.emplace_back(cv.to_bytes(value));
		}
	}
	std::tie(ent, acc) =
		make_entity_and_account_2(
			this->_fields.begin(),
			this->_fields.end(),
			responses.begin(),
			"6",
			0,
			this->_minentropy
		);
	return std::make_tuple(ent, acc);
}

#ifndef GGG_DISABLE_PAM
std::tuple<ggg::entity,ggg::account>
ggg::form
::input_entity(ggg::pam_handle* pamh) {
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
		std::vector<bool> status = this->validate(r);
		valid = std::find(status.begin(), status.end(), false) == status.end();
		if (valid) {
			std::tie(ent, acc) = make_entity_and_account(r, pamh);
		} else {
			std::stringstream msg;
			msg << "Invalid fields: ";
			bool comma = false;
			const size_t n = status.size();
			for (size_t i=0; i<n; ++i) {
				const form_field& ff = this->_fields[i];
				if (!status[i]) {
					if (comma) {
						msg << ", ";
					} else {
						comma = true;
					}
					msg << ff.name();
				}
			}
			std::string s = msg.str();
			conv->error(s.data());
			pamh->debug("%s", s.data());
		}
		/*
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
		 */
	} while (!valid);
	return std::make_tuple(ent, acc);
}

std::vector<bool>
ggg::form
::validate(const responses& r) {
	std::vector<bool> status(r.size());
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	const size_t n = r.size();
	for (size_t i=0; i<n; ++i) {
		const response& resp = r[i];
		const form_field& ff = this->_fields[i];
		bool valid = true;
		if (ff.is_input()) {
			std::wregex expr(cv.from_bytes(ff.regex()));
			std::wstring field_value = cv.from_bytes(resp.text());
			valid = std::regex_match(field_value, expr);
		}
		status[i] = valid;
	}
	return status;
}

std::tuple<ggg::entity,ggg::account>
ggg::form
::make_entity_and_account(const responses& r, ggg::pam_handle* pamh) {
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
					ggg::validate_password(new_password, this->_minentropy);
					ggg::secure_string encrypted =
						ggg::encrypt(
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

#endif // ifndef GGG_DISABLE_PAM

std::string
ggg
::interpolate(std::string orig, const field_values& values, char prefix) {
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
		    st == parsing_close_bracket) {
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
