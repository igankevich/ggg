#include <ostream>

#include <ggg/core/form.hh>

void
ggg::operator>>(const sqlite::statement& in, form2& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	cstr >> rhs._name;
	cstr >> rhs._content;
}


std::ostream&
ggg::operator<<(std::ostream& out, const form2& rhs) {
	return out << rhs.name();
}
