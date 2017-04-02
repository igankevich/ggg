#include "entity.hh"

std::istream&
legion::operator>>(std::istream& in, entity& rhs) {
	std::istream::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, ':',
			rhs._username,
			rhs._password,
			rhs._uid,
			rhs._gid,
			rhs._realname,
			rhs._homedir,
			rhs._shell
		);
	}
	return in;
}

std::ostream&
legion::operator<<(std::ostream& out, const entity& rhs) {
	return out
		<< rhs.name() << ':'
		<< rhs.password() << ':'
		<< rhs.id() << ':'
		<< rhs.group_id() << ':'
		<< rhs.real_name() << ':'
		<< rhs.home() << ':'
		<< rhs.shell();
}
