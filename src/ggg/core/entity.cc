#include <ggg/bits/read_field.hh>
#include <ggg/core/entity.hh>

#include <algorithm>
#include <iomanip>
#include <sstream>

std::istream&
ggg::operator>>(std::istream& in, entity& rhs) {
	typename std::istream::sentry s(in);
	if (s) {
		rhs.clear();
        ggg::bits::ignore_field ignore;
		bits::read_all_fields(
			in,
			entity::delimiter,
			rhs._name,
            ignore,
			rhs._id,
            ignore,
			rhs._description,
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
	       << 'x' << entity::delimiter
	       << rhs.id() << entity::delimiter
	       << rhs.id() << entity::delimiter
	       << rhs.description() << entity::delimiter
	       << rhs.home() << entity::delimiter
	       << rhs.shell();
}

sys::bstream&
ggg::operator<<(sys::bstream& out, const entity& rhs) {
	return out << rhs._name
	           << rhs._description
	           << rhs._homedir
	           << rhs._shell
	           << rhs._id;
}

sys::bstream&
ggg::operator>>(sys::bstream& in, entity& rhs) {
	rhs.clear();
	return in
	       >> rhs._name
	       >> rhs._description
	       >> rhs._homedir
	       >> rhs._shell
	       >> rhs._id;
}

bool
ggg::entity
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

void
ggg::entity::clear() {
	this->_name.clear();
	this->_description.clear();
	this->_homedir.clear();
	this->_shell.clear();
	this->_id = -1;
	this->_parent.clear();
}

ggg::entity&
ggg::entity::operator=(const struct ::passwd& rhs) {
	this->_name = rhs.pw_name;
	this->_description = rhs.pw_gecos;
	this->_homedir = rhs.pw_dir;
	this->_shell = rhs.pw_shell;
	this->_id = rhs.pw_uid;
    return *this;
}

void
ggg::operator>>(const sqlite::statement& in, entity& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	int64_t id = -1;
	cstr >> id;
	cstr >> rhs._name;
	cstr >> rhs._description;
	cstr >> rhs._homedir;
	cstr >> rhs._shell;
	rhs._id = static_cast<sys::uid_type>(id);
}
