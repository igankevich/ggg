#include <ostream>
#include <iomanip>

#include <ggg/core/machine.hh>

template <>
char
ggg::Entity_header<ggg::Machine>::delimiter() {
	return ' ';
}

template <>
void
ggg::Entity_header<ggg::Machine>::write_header(
	std::ostream& out,
	width_container width,
	char delim
) {
	if (!width) {
		out << "NAME HWADDR IPADDR\n";
		return;
	}
	const char d[4] = {' ', delim, ' ', 0};
	out << std::left << std::setw(width[0]) << "NAME" << d
		<< std::left << std::setw(width[3]) << "HWADDR" << d
		<< std::left << std::setw(width[4]) << "IPADDR" << '\n';
}

template <>
void
ggg::Entity_header<ggg::Machine>::write_body(
	std::ostream& out,
	const Machine& ent,
	width_container width,
	entity_format fmt,
	char delim
) {
	const char d[4] = {' ', delim, ' ', 0};
	out << std::left << std::setw(width[0]) << ent.name() << d
		<< std::left << std::setw(width[1]) << ent.ethernet_address() << d
		<< std::left << std::setw(width[2]) << ent.address() << '\n';
}
