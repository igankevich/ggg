#include <ggg/core/public_key.hh>

void
ggg::public_key::clear() {
    this->_name.clear();
    this->_options.clear();
    this->_type.clear();
    this->_key.clear();
    this->_comment.clear();
}

void
ggg::operator>>(const sqlite::statement& in, public_key& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	cstr >> rhs._name >> rhs._options >> rhs._type
        >> rhs._key >> rhs._comment;
}

std::ostream&
ggg::operator<<(std::ostream& out, const public_key& rhs) {
    if (!rhs.options().empty()) { out << rhs.options() << ' '; }
    out << rhs.type() << ' ';
    out << rhs.key() << ' ';
    out << rhs.comment();
    return out;
}

