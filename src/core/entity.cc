#include "entity.hh"
#include "bits/read_field.hh"
#include "bits/bufcopy.hh"
#include <iomanip>
#include <algorithm>

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
ggg::entity::write_human(std::ostream& out, columns_type width) const {
	const char d[4] = {' ', delimiter, ' ', 0};
	out << std::left << std::setw(width[0]) << this->name() << d
		<< std::right << std::setw(width[2]) << this->id() << d
		<< std::right << std::setw(width[3]) << this->gid() << d
		<< std::left << std::setw(width[4]) << this->real_name() << d
		<< std::left << std::setw(width[5]) << this->home() << d
		<< std::left << std::setw(width[6]) << this->shell() << '\n';
}

std::istream&
ggg::entity::read_human(std::istream& in) {
	std::istream::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, entity::delimiter,
			this->_name,
			this->_uid,
			this->_gid,
			this->_realname,
			this->_homedir,
			this->_shell
		);
		if (in.eof()) {
			in.clear();
		}
	}
	return in;
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

bool
ggg::entity::has_valid_name() const noexcept {
	bool ret = !this->_name.empty();
	if (ret) {
		ret = std::all_of(
			this->_name.begin(),
			this->_name.end(),
			[] (char ch) {
				return (ch >= 'a' && ch <= 'z')
					|| (ch >= 'A' && ch <= 'Z')
					|| (ch >= '0' && ch <= '9')
					|| ch == '-' || ch == '.' || ch == '_';
			}
		);
	}
	// not composed entirely of digits
	if (ret) {
		ret = !std::all_of(
			this->_name.begin(),
			this->_name.end(),
			[] (char ch) {
				return ch >= '0' && ch <= '9';
			}
		);
	}
	// starts with a character
	if (ret) {
		const char ch = this->_name.front();
		ret = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
	}
	return ret;
}

void
ggg::entity::clear() {
	this->_name.clear();
	this->_password.clear();
	this->_realname.clear();
	this->_homedir.clear();
	this->_shell.clear();
	this->_uid = -1;
	this->_gid = -1;
	this->_origin = sys::path();
}
