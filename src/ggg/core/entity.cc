#include <ggg/bits/read_field.hh>
#include <ggg/bits/to_bytes.hh>
#include <ggg/core/entity.hh>

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace {

	template <class String>
	inline void
	merge_str(String& lhs, const String& rhs) {
		if (lhs.empty() && !rhs.empty()) {
			lhs = rhs;
		}
	}

	template <class String>
	inline void
	merge_origin(String& lhs, const String& rhs) {
		if (!rhs.empty()) {
			lhs = rhs;
		}
	}

	template <class Ch>
	struct entity_headers;

	template <>
	struct entity_headers<char> {

		typedef char char_type;

		inline static const char_type*
		name() noexcept {
			return "USERNAME";
		}

		inline static const char_type*
		id() noexcept {
			return "ID";
		}

		inline static const char_type*
		real_name() noexcept {
			return "REALNAME";
		}

		inline static const char_type*
		home() noexcept {
			return "HOME";
		}

		inline static const char_type*
		shell() noexcept {
			return "SHELL";
		}

		inline static const char_type*
		all() noexcept {
			return "USERNAME:unused:ID:unused:REALNAME:HOME:SHELL";
		}

	};

}

template <class Ch>
std::basic_istream<Ch>&
ggg::operator>>(std::basic_istream<Ch>& in, basic_entity<Ch>& rhs) {
	typename std::basic_istream<Ch>::sentry s(in);
	if (s) {
		rhs.clear();
		bits::read_all_fields(
			in,
			basic_entity<Ch>::delimiter,
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
sys::basic_bstream<Ch>&
ggg::operator<<(sys::basic_bstream<Ch>& out, const basic_entity<Ch>& rhs) {
	return out << rhs._name
	           << rhs._password
	           << rhs._realname
	           << rhs._homedir
	           << rhs._shell
	           << rhs._uid
	           << rhs._gid;
}

template <class Ch>
sys::basic_bstream<Ch>&
ggg::operator>>(sys::basic_bstream<Ch>& in, basic_entity<Ch>& rhs) {
	rhs.clear();
	return in
	       >> rhs._name
	       >> rhs._password
	       >> rhs._realname
	       >> rhs._homedir
	       >> rhs._shell
	       >> rhs._uid
	       >> rhs._gid;
}

template <class Ch>
bool
ggg::basic_entity<Ch>
::has_valid_name() const noexcept {
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
ggg::basic_entity<Ch>
::clear() {
	this->_name.clear();
	this->_password.clear();
	this->_realname.clear();
	this->_homedir.clear();
	this->_shell.clear();
	this->_uid = -1;
	this->_gid = -1;
	this->_parent.clear();
}

template <class Ch>
void
ggg::basic_entity<Ch>
::merge(const basic_entity& rhs) {
	if (this->_name != rhs._name) {
		return;
	}
	merge_str(this->_password, rhs._password);
	merge_str(this->_realname, rhs._realname);
	merge_str(this->_homedir, rhs._homedir);
	merge_str(this->_shell, rhs._shell);
	merge_str(this->_parent, rhs._parent);
	if (!this->has_id() && rhs.has_id()) {
		this->_uid = rhs._uid;
	}
	if (!this->has_gid() && rhs.has_gid()) {
		this->_gid = rhs._gid;
	}
}

template <class Ch>
void
ggg
::assign(basic_entity<Ch>& lhs, const struct ::passwd& rhs) {
	lhs._name = rhs.pw_name;
	lhs._password = rhs.pw_passwd;
	lhs._realname = rhs.pw_gecos;
	lhs._homedir = rhs.pw_dir;
	lhs._shell = rhs.pw_shell;
	lhs._uid = rhs.pw_uid;
	lhs._gid = rhs.pw_gid;
}

void
ggg::operator>>(const sqlite::statement& in, basic_entity<char>& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	int64_t id = -1;
	cstr >> id;
	cstr >> rhs._name;
	cstr >> rhs._realname;
	cstr >> rhs._homedir;
	cstr >> rhs._shell;
	rhs._uid = static_cast<sys::uid_type>(id);
	rhs._gid = static_cast<sys::gid_type>(id);
}

template class ggg::basic_entity<char>;

template void
ggg
::assign(basic_entity<char>& lhs, const struct ::passwd& rhs);

template std::basic_istream<char>&
ggg::operator>>(std::basic_istream<char>&, basic_entity<char>&);

template std::basic_ostream<char>&
ggg::operator<<(std::basic_ostream<char>&, const basic_entity<char>&);

template sys::basic_bstream<char>&
ggg::operator<<(sys::basic_bstream<char>&, const basic_entity<char>&);

template sys::basic_bstream<char>&
ggg::operator>>(sys::basic_bstream<char>&, basic_entity<char>&);
