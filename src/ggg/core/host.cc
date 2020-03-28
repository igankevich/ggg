#include <cstring>
#include <ostream>

#include <ggg/nss/buffer.hh>
#include <ggg/core/host.hh>

std::ostream&
ggg::operator<<(std::ostream& out, const host& rhs) {
    return out << rhs.address() << ' ' << rhs.name();
}

void
ggg::operator>>(const sqlite::statement& in, host& rhs) {
    sqlite::cstream cstr(in);
    cstr >> rhs._address >> rhs._name;
}

sqlite::cstream&
ggg::operator>>(sqlite::cstream& in, sys::ethernet_address& rhs) {
    sqlite::blob addr;
    in >> addr;
    if (addr.size() == rhs.size()) {
        std::copy_n(addr.data(), addr.size(), rhs.data());
    }
    return in;
}
