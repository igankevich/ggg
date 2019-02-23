#include <cstring>
#include <ostream>

#include <ggg/bits/bufcopy.hh>
#include <ggg/core/host.hh>

std::ostream&
ggg::operator<<(std::ostream& out, const host& rhs) {
	return out << rhs.address() << ' ' << rhs.name();
}

sqlite::rstream&
ggg::operator>>(sqlite::rstream& in, host& rhs) {
	sqlite::cstream cstr(in);
	sqlite::blob addr;
	if (in >> cstr) {
		cstr >> addr;
		if (addr.size() == rhs._address.size()) {
			std::copy_n(
				addr.data(),
				addr.size(),
				rhs._address.data()
			);
		}
		cstr >> rhs._name;
	}
	return in;
}

void
ggg::copy_to(const host& ent, struct ::etherent* lhs, char* buffer) {
	buffer = bits::bufcopy(&lhs->e_name, buffer, ent.name().data());
	std::memcpy(
		lhs->e_addr.ether_addr_octet,
		ent.address().data(),
		ent.address().size()
	);
}

