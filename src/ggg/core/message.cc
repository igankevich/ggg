#include <locale>

#include <ggg/core/message.hh>

void
ggg::message::clear() {
    this->_name.clear();
    this->_hostname.clear();
    this->_text.clear();
    this->_timestamp = time_point(duration::zero());
}

void
ggg::operator>>(const sqlite::statement& in, message& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	cstr >> rhs._name >> rhs._timestamp >> rhs._hostname >> rhs._text;
}

std::ostream&
ggg::operator<<(std::ostream& out, const message& rhs) {
    auto t = message::clock_type::to_time_t(rhs._timestamp);
    char buf[25];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", std::localtime(&t));
	return out
		<< buf << message::delimiter
		<< rhs._hostname << message::delimiter
		<< rhs._name << message::delimiter
		<< rhs._text;
}

