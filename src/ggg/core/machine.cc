#include <ostream>

#include <ggg/core/machine.hh>

std::ostream&
ggg::operator<<(std::ostream& out, const Machine& rhs) {
	return out
		<< rhs._name << ' '
		<< rhs._ethernet_address << ' '
		<< rhs._ip_address;
}

sqlite::rstream&
ggg::operator>>(sqlite::rstream& in, Machine& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	if (in >> cstr) {
		cstr >> rhs._name >> rhs._ip_address >> rhs._ethernet_address;
	}
	return in;
}

