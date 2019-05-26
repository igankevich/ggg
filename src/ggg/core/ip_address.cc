#include <cstring>
#include <ostream>

#include <ggg/core/ip_address.hh>

ggg::ip_address::ip_address(const ip_address& rhs):
_family{rhs._family} {
	switch (rhs.family()) {
		case sys::family_type::inet:
			this->_address4 = rhs._address4;
			break;
		case sys::family_type::inet6:
			this->_address6 = rhs._address6;
			break;
		default:
			break;
	}
}

ggg::ip_address&
ggg::ip_address::operator=(const ip_address& rhs) {
	this->_family = rhs._family;
	switch (rhs.family()) {
		case sys::family_type::inet:
			this->_address4 = rhs._address4;
			break;
		case sys::family_type::inet6:
			this->_address6 = rhs._address6;
			break;
		default:
			break;
	}
	return *this;
}

bool
ggg::operator==(const ip_address& lhs, const ip_address& rhs) {
	if (lhs._family != rhs._family) {
		return false;
	}
	switch (rhs.family()) {
		case sys::family_type::inet: return lhs._address4 == rhs._address4;
		case sys::family_type::inet6: return lhs._address6 == rhs._address6;
		default: return false;
	}
}

bool
ggg::operator<(const ip_address& lhs, const ip_address& rhs) {
	if (lhs._family != rhs._family) {
		return lhs._family < rhs._family;
	}
	switch (rhs.family()) {
		case sys::family_type::inet: return lhs._address4 < rhs._address4;
		case sys::family_type::inet6: return lhs._address6 < rhs._address6;
		default: return false;
	}
}

std::istream&
ggg::operator>>(std::istream& in, ip_address& rhs) {
	rhs.clear();
	auto oldg = in.tellg();
	if (in >> rhs._address4) {
		rhs._family = sys::family_type::inet;
	} else {
		in.seekg(oldg);
		if (in >> rhs._address6) {
			rhs._family = sys::family_type::inet6;
		}
	}
	return in;
}

std::ostream&
ggg::operator<<(std::ostream& out, const ip_address& rhs) {
	switch (rhs.family()) {
		case sys::family_type::inet:
			out << rhs._address4;
			break;
		case sys::family_type::inet6:
			out << rhs._address6;
			break;
		default:
			out << "unknown";
			break;
	}
	return out;
}

void
ggg::operator>>(const sqlite::statement& in, ip_address& rhs) {
	sqlite::cstream cstr(in);
	cstr >> rhs;
}

sqlite::cstream&
ggg::operator>>(sqlite::cstream& in, ip_address& rhs) {
	rhs.clear();
	sqlite::blob addr;
	in >> addr;
	void* ptr = nullptr;
	if (addr.size() == sizeof(rhs._address4)) {
		rhs._family = sys::family_type::inet;
		ptr = rhs._address4.data();
	} else if (addr.size() == sizeof(rhs._address6)) {
		rhs._family = sys::family_type::inet6;
		ptr = rhs._address6.data();
	}
	if (ptr) { std::memcpy(ptr, addr.data(), addr.size()); }
	return in;
}

