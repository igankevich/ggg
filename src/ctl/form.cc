#include "form.hh"

#include <algorithm>
#include <codecvt>
#include <fstream>
#include <iterator>
#include <locale>
#include <regex>
#include <sstream>
#include <string>

#include "config.hh"
#include <pam/conversation.hh>

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
	if (in.is_open()) {
		std::copy(
			std::istream_iterator<form_field>(in),
			std::istream_iterator<form_field>(),
			std::back_inserter(this->all_fields)
		);
		std::sort(this->all_fields.begin(), this->all_fields.end());
	}
}

ggg::entity
ggg::form::input_entity(ggg::pam_handle* pamh) {
	messages m;
	init_messages(this->all_fields, m, this->is_console());
	conversation_ptr conv = pamh->get_conversation();
	entity ent;
	bool valid;
	do {
		responses r(m.size());
		conv->converse(m, r);
		valid = this->validate(r);
		if (valid) {
			ent = make_entity(r);
		}
		std::stringstream msg;
		msg << "fields=";
		std::copy(
			this->all_fields.begin(),
			this->all_fields.end(),
			std::ostream_iterator<form_field>(msg, ",")
		);
		msg << "responses=" << r << ",ent=" << ent << ",valid=" << valid;
		pamh->debug("%s", msg.str().data());
	} while (!valid);
	conv->prompt("press any key...");
	return ent;
}

bool
ggg::form::validate(const responses& r) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	return std::equal(
		this->all_fields.begin(), this->all_fields.end(), r.begin(),
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

ggg::entity
ggg::form::make_entity(const responses& r) {
	field_values values;
	entity ent;
	auto first = this->all_fields.begin();
	auto last = this->all_fields.end();
	auto first2 = r.begin();
	while (first != last) {
		if (first->type() == field_type::set) {
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

