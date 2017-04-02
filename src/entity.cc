#include "entity.hh"
#include "read_field.hh"
#include "bufcopy.hh"

std::istream&
legion::operator>>(std::istream& in, entity& rhs) {
	std::istream::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, ':',
			rhs._name,
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

void
legion::entity::copy_to(struct ::passwd* lhs, char* buffer) const {
	buffer = bits::bufcopy(&lhs->pw_name, buffer, this->_name.data());
	buffer = bits::bufcopy(&lhs->pw_passwd, buffer, this->_password.data());
	#ifdef __linux__
	buffer = bits::bufcopy(&lhs->pw_gecos, buffer, this->_realname.data());
	#endif
	buffer = bits::bufcopy(&lhs->pw_dir, buffer, this->_homedir.data());
	buffer = bits::bufcopy(&lhs->pw_shell, buffer, this->_shell.data());
	lhs->pw_uid = this->_uid;
	lhs->pw_gid = this->_gid;
}
