#include "form_field.hh"

#include "bits/read_field.hh"

#include <stdexcept>
#include <limits>

std::istream&
ggg::operator>>(std::istream& in, form_field& rhs) {
	std::istream::sentry s(in);
	if (s) {
		rhs.clear();
		std::string prefix;
		char last = bits::read_field(in, prefix, form_field::delimiter);
		rhs._type = to_field_type(prefix);
		if (last == form_field::delimiter) {
			if (rhs.is_constant()) {
				rhs._id = std::numeric_limits<form_field::id_type>::max();
				bits::read_all_fields(
					in, form_field::delimiter,
					rhs._target,
					rhs._value
				);
			} else {
				bits::read_all_fields(
					in, form_field::delimiter,
					rhs._id,
					rhs._name,
					rhs._target,
					rhs._regex
				);
			}
		}
		if (in.eof()) {
			in.clear();
		}
	}
	return in;
}

std::ostream&
ggg::operator<<(std::ostream& out, const form_field& rhs) {
	if (!rhs.is_constant()) {
		out << rhs._id << form_field::delimiter;
	}
	out << rhs._name << form_field::delimiter
		<< rhs._target << form_field::delimiter
		<< rhs._regex;
	return out;
}

void
ggg::form_field::clear() {
	this->_id = 0;
	this->_name.clear();
	this->_regex.clear();
	this->_target.clear();
	this->_value.clear();
	this->_type = field_type::text;
}

void
ggg::swap(form_field& lhs, form_field& rhs) {
	std::swap(lhs._id, rhs._id);
	std::swap(lhs._name, rhs._name);
	std::swap(lhs._regex, rhs._regex);
	std::swap(lhs._target, rhs._target);
	std::swap(lhs._value, rhs._value);
	std::swap(lhs._type, rhs._type);
}

ggg::field_type
ggg::to_field_type(const std::string& s) {
	field_type t;
	if (s == "const") {
		t = field_type::constant;
	} else if (s == "text") {
		t = field_type::text;
	} else if (s == "password") {
		t = field_type::password;
	} else {
		throw std::invalid_argument("bad field type");
	}
	return t;
}

std::istream&
ggg::operator>>(std::istream& in, field_type& rhs) {
	std::string s;
	in >> s;
	rhs = to_field_type(s);
	return in;
}

std::ostream&
ggg::operator<<(std::ostream& out, field_type rhs) {
	const char* s;
	switch (rhs) {
		case field_type::text: s = "text"; break;
		case field_type::password: s = "password"; break;
		case field_type::constant: s = "const"; break;
		default: s = "unknown"; break;
	}
	out << s;
	return out;
}
