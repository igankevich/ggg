#include "entity.hh"
#include "bits/read_field.hh"
#include "bits/bufcopy.hh"
#include <iomanip>

std::istream&
ggg::operator>>(std::istream& in, entity& rhs) {
	std::istream::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, entity::delimiter,
			rhs._name,
			rhs._password,
			rhs._uid,
			rhs._gid,
			rhs._realname,
			rhs._homedir,
			rhs._shell
		);
		if (in.eof()) {
			in.clear();
		}
	}
	return in;
}

std::ostream&
ggg::operator<<(std::ostream& out, const entity& rhs) {
	return out
		<< rhs.name() << entity::delimiter
		<< rhs.password() << entity::delimiter
		<< rhs.id() << entity::delimiter
		<< rhs.gid() << entity::delimiter
		<< rhs.real_name() << entity::delimiter
		<< rhs.home() << entity::delimiter
		<< rhs.shell();
}

void
ggg::entity::print_aligned(std::ostream& out, columns_type width) const {
	const char d[4] = {' ', delimiter, ' ', 0};
	out << std::left << std::setw(width[0]) << this->name() << d
		<< std::left << std::setw(width[1]) << this->password() << d
		<< std::right << std::setw(width[2]) << this->id() << d
		<< std::right << std::setw(width[3]) << this->gid() << d
		<< std::left << std::setw(width[4]) << this->real_name() << d
		<< std::left << std::setw(width[5]) << this->home() << d
		<< std::left << std::setw(width[6]) << this->shell() << '\n';
}

size_t
ggg::entity::buffer_size() const noexcept {
	return this->_name.size() + 1
		+ this->_password.size() + 1
		+ this->_realname.size() + 1
		+ this->_homedir.size() + 1
		+ this->_shell.size() + 1;
}

void
ggg::entity::copy_to(struct ::passwd* lhs, char* buffer) const {
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
