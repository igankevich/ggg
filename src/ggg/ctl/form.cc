#include "form.hh"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>

#include <ggg/config.hh>
#include <ggg/ctl/password.hh>


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
		if (ff.type() == field_type::set) {
			if (ff.target() == "entropy") {
				try {
					this->_minentropy = std::max(0.0, std::stod(ff.regex()));
				} catch (const std::exception& err) {
					std::cerr << std::endl << "bad entropy" << std::endl;
					throw;
				}
			} else if (ff.target() == "locale") {
				try {
					this->_locale = std::locale(ff.regex());
					std::clog << "locale: " << this->_locale.name() << std::endl;
				} catch (const std::exception& err) {
					std::cerr << std::endl << "bad locale" << std::endl;
					throw;
				}
			}
		}
	}
}

std::tuple<ggg::entity,ggg::account>
ggg::form
::make_entity_and_account(std::vector<std::string> responses) {
	field_values values;
	entity ent;
	account acc;
	auto first = this->_fields.begin();
	auto last = this->_fields.end();
	auto first2 = responses.begin();
	std::string password_id = "6";
	unsigned int num_rounds = 0;
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
