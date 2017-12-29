#include "entity.hh"
#include "bits/read_field.hh"
#include "bits/bufcopy.hh"
#include <iomanip>
#include <algorithm>
#include <sstream>

namespace {

	template <class String>
	inline void
	merge_str(String& lhs, const String& rhs) {
		if (lhs.empty() && !rhs.empty()) {
			lhs = rhs;
		}
	}

}

template <class Ch>
ggg::basic_entity<Ch>::basic_entity(const basic_entity<char>& rhs, wcvt_type& cv):
_name(cv.from_bytes(rhs.name())),
_password(cv.from_bytes(rhs.password())),
_realname(cv.from_bytes(rhs.real_name())),
_homedir(cv.from_bytes(rhs.home())),
_shell(cv.from_bytes(rhs.shell())),
_uid(rhs.id()),
_gid(rhs.gid()),
_origin(cv.from_bytes(rhs.origin()))
{}

template <class Ch>
std::basic_istream<Ch>&
ggg::operator>>(std::basic_istream<Ch>& in, basic_entity<Ch>& rhs) {
	typename std::basic_istream<Ch>::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, basic_entity<Ch>::delimiter,
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

template <class Ch>
std::basic_ostream<Ch>&
ggg::operator<<(std::basic_ostream<Ch>& out, const basic_entity<Ch>& rhs) {
	return out
		<< rhs.name() << basic_entity<Ch>::delimiter
		<< rhs.password() << basic_entity<Ch>::delimiter
		<< rhs.id() << basic_entity<Ch>::delimiter
		<< rhs.gid() << basic_entity<Ch>::delimiter
		<< rhs.real_name() << basic_entity<Ch>::delimiter
		<< rhs.home() << basic_entity<Ch>::delimiter
		<< rhs.shell();
}

template <class Ch>
void
ggg::basic_entity<Ch>::write_human(ostream_type& out, columns_type width) const {
	const char d[4] = {' ', delimiter, ' ', 0};
	out << std::left << std::setw(width[0]) << this->name() << d
		<< std::right << std::setw(width[2]) << this->id() << d
		<< std::right << std::setw(width[3]) << this->gid() << d
		<< std::left << std::setw(width[4]) << this->real_name() << d
		<< std::left << std::setw(width[5]) << this->home() << d
		<< std::left << std::setw(width[6]) << this->shell() << '\n';
}

template <class Ch>
std::basic_istream<Ch>&
ggg::basic_entity<Ch>::read_human(std::basic_istream<Ch>& in) {
	typename std::basic_istream<Ch>::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, basic_entity<Ch>::delimiter,
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

template <class Ch>
size_t
ggg::buffer_size(const basic_entity<Ch>& rhs) noexcept {
	return rhs._name.size() + 1
		+ rhs._password.size() + 1
		+ rhs._realname.size() + 1
		+ rhs._homedir.size() + 1
		+ rhs._shell.size() + 1;
}

template <class Ch>
void
ggg::copy_to(
	const basic_entity<Ch>& ent,
	struct ::passwd* lhs,
	char* buffer
) {
	buffer = bits::bufcopy(&lhs->pw_name, buffer, ent._name.data());
	buffer = bits::bufcopy(&lhs->pw_passwd, buffer, ent._password.data());
	#ifdef __linux__
	buffer = bits::bufcopy(&lhs->pw_gecos, buffer, ent._realname.data());
	#endif
	buffer = bits::bufcopy(&lhs->pw_dir, buffer, ent._homedir.data());
	buffer = bits::bufcopy(&lhs->pw_shell, buffer, ent._shell.data());
	lhs->pw_uid = ent._uid;
	lhs->pw_gid = ent._gid;
}

template <class Ch>
bool
ggg::basic_entity<Ch>::has_valid_name() const noexcept {
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

template <class Ch>
void
ggg::basic_entity<Ch>::clear() {
	this->_name.clear();
	this->_password.clear();
	this->_realname.clear();
	this->_homedir.clear();
	this->_shell.clear();
	this->_uid = -1;
	this->_gid = -1;
	this->_origin.clear();
}

template <class Ch>
void
ggg::basic_entity<Ch>::set(const form_field& field, const Ch* value) {
	typedef std::basic_stringstream<char_type, traits_type> sstream_type;
	const std::string& t = field.target();
	if (t == "basic_entity.name") {
		this->_name = value;
	} else if (t == "basic_entity.realname") {
		this->_realname = value;
	} else if (t == "basic_entity.homedir") {
		this->_homedir = value;
	} else if (t == "basic_entity.shell") {
		this->_shell = value;
	} else if (t == "basic_entity.uid") {
		sstream_type str(value);
		str >> this->_uid;
		if (str.fail()) {
			throw std::invalid_argument("bad uid");
		}
	} else if (t == "basic_entity.gid") {
		sstream_type str(value);
		str >> this->_gid;
		if (str.fail()) {
			throw std::invalid_argument("bad gid");
		}
	} else if (t == "basic_entity.origin") {
		this->_origin = value;
	} else {
		throw std::invalid_argument("bad field target");
	}
}

template <class Ch>
void
ggg::basic_entity<Ch>::merge(const basic_entity& rhs) {
	if (this->_name != rhs._name) {
		return;
	}
	merge_str(this->_password, rhs._password);
	merge_str(this->_realname, rhs._realname);
	merge_str(this->_homedir, rhs._homedir);
	merge_str(this->_shell, rhs._shell);
	merge_str(this->_origin, rhs._origin);
	if (!this->has_id() && rhs.has_id()) {
		this->_uid = rhs._uid;
	}
	if (!this->has_gid() && rhs.has_gid()) {
		this->_gid = rhs._gid;
	}
}

template <class Ch>
void
ggg::assign(basic_entity<Ch>& lhs, const struct ::passwd& rhs) {
	lhs._name = rhs.pw_name;
	lhs._password = rhs.pw_passwd;
	lhs._realname = rhs.pw_gecos;
	lhs._homedir = rhs.pw_dir;
	lhs._shell = rhs.pw_shell;
	lhs._uid = rhs.pw_uid;
	lhs._gid = rhs.pw_gid;
}

template class ggg::basic_entity<char>;
template class ggg::basic_entity<wchar_t>;

template void
ggg::copy_to(const basic_entity<char>& ent, struct ::passwd* lhs, char* buffer);

template size_t
ggg::buffer_size(const basic_entity<char>& ent) noexcept;

template void
ggg::assign(basic_entity<char>& lhs, const struct ::passwd& rhs);

template std::basic_istream<char>&
ggg::operator>>(std::basic_istream<char>& in, basic_entity<char>& rhs);

template std::basic_istream<wchar_t>&
ggg::operator>>(std::basic_istream<wchar_t>& in, basic_entity<wchar_t>& rhs);

template std::basic_ostream<char>&
ggg::operator<<(std::basic_ostream<char>& out, const basic_entity<char>& rhs);

template std::basic_ostream<wchar_t>&
ggg::operator<<(std::basic_ostream<wchar_t>& out, const basic_entity<wchar_t>& rhs);
