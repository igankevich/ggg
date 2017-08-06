#include "form_field.hh"

#include "bits/read_field.hh"

#include <cstdlib>

std::istream&
ggg::operator>>(std::istream& in, form_field& rhs) {
	std::istream::sentry s(in);
	if (s) {
		rhs.clear();
		std::string prefix;
		char last = bits::read_field(in, prefix, form_field::delimiter);
		if (prefix == "const") {
			rhs._isconstant = true;
			rhs._id = 0;
		} else {
			rhs._id = std::strtoul(prefix.data(), nullptr, 10);
		}
		if (last == form_field::delimiter) {
			if (rhs._isconstant) {
				bits::read_all_fields(
					in, form_field::delimiter,
					rhs._target,
					rhs._value
				);
			} else {
				bits::read_all_fields(
					in, form_field::delimiter,
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
	if (rhs._isconstant) {
		out << "const" << form_field::delimiter
			<< rhs._target << form_field::delimiter
			<< rhs._value << form_field::delimiter;
	} else {
		out << rhs._id << form_field::delimiter
			<< rhs._name << form_field::delimiter
			<< rhs._target << form_field::delimiter
			<< rhs._regex;
	}
	return out;
}

void
ggg::form_field::clear() {
	this->_id = 0;
	this->_name.clear();
	this->_regex.clear();
	this->_target.clear();
	this->_value.clear();
	this->_isconstant = false;
}

void
ggg::swap(form_field& lhs, form_field& rhs) {
	std::swap(lhs._id, rhs._id);
	std::swap(lhs._name, rhs._name);
	std::swap(lhs._regex, rhs._regex);
	std::swap(lhs._target, rhs._target);
	std::swap(lhs._value, rhs._value);
	std::swap(lhs._isconstant, rhs._isconstant);
}
