#include <ostream>

#include <ggg/core/host_address.hh>

std::ostream&
ggg::operator<<(std::ostream& out, const host_address& rhs) {
    return out << rhs._address << ' ' << rhs._name;
}

void
ggg::operator>>(const sqlite::statement& in, host_address& rhs) {
    rhs.clear();
    sqlite::cstream cstr(in);
    cstr >> rhs._address >> rhs._name;
}
