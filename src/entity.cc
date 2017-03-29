#include "entity.hh"

namespace {

	inline const char*
	non_null(const char* rhs) {
		return rhs ? rhs : "";
	}

}

std::istream&
legion::operator>>(std::istream& in, entity& rhs) {
	std::istream::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, ':',
			rhs.pw_name,
			rhs.pw_passwd,
			rhs.pw_uid,
			rhs.pw_gid,
			#ifdef __linux__
			rhs.pw_gecos,
			#endif
			rhs.pw_dir,
			rhs.pw_shell
		);
	}
	return in;
}

std::ostream&
legion::operator<<(std::ostream& out, const entity& rhs) {
	return out
		<< non_null(rhs.name()) << ':'
		<< non_null(rhs.password()) << ':'
		<< rhs.id() << ':'
		<< rhs.group_id() << ':'
		<< non_null(rhs.real_name()) << ':'
		<< non_null(rhs.home()) << ':'
		<< non_null(rhs.shell());
}
