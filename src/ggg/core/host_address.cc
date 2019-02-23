#include <ostream>

#include <ggg/core/host_address.hh>

std::ostream&
ggg::operator<<(std::ostream& out, const host_address& rhs) {
	return out << rhs._address << ' ' << rhs._name;
}

sqlite::rstream&
ggg::operator>>(sqlite::rstream& in, host_address& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	if (in >> cstr) {
		cstr >> rhs._address >> rhs._name;
	}
	return in;
}
