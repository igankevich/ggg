#include "form_type.hh"

#include <istream>
#include <ostream>

std::ostream&
ggg::operator<<(std::ostream& out, form_type rhs) {
	switch (rhs) {
		case form_type::console: out << "console"; break;
		case form_type::graphical: out << "gui"; break;
	}
	return out;
}

std::istream&
ggg::operator>>(std::istream& in, form_type& rhs) {
	std::string s;
	if (in >> s) {
		rhs = from_string(s);
	}
	return in;
}

ggg::form_type
ggg::from_string(const std::string& rhs) {
	form_type tp;
	if (rhs == "console") {
		tp = form_type::console;
	} else if (rhs == "gui") {
		tp = form_type::graphical;
	} else {
		throw std::invalid_argument("bad form type");
	}
	return tp;
}

