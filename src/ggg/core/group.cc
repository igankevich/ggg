#include "group.hh"

#include <cstring>
#include <memory>
#include <unistdx/it/intersperse_iterator>

namespace ggg {

    template <class Container>
    sys::bstream&
    operator<<(sys::bstream& out, const Container& rhs) {
        out << uint32_t(rhs.size());
        for (const auto& elem : rhs) {
            out << elem;
        }
        return out;
    }

    template <class T>
    sys::bstream&
    operator>>(sys::bstream& in, std::unordered_set<T>& rhs) {
        rhs.clear();
        uint32_t n = 0;
        in >> n;
        for (uint32_t i=0; i<n; ++i) {
            T elem;
            in >> elem;
            rhs.emplace(elem);
        }
        return in;
    }

}

std::ostream&
ggg::operator<<(std::ostream& out, const group& rhs) {
    out << rhs.name() << "::" << rhs.id() << ':';
    std::copy(
        rhs._members.begin(),
        rhs._members.end(),
        sys::intersperse_iterator<std::string,char,char>(out, ',')
    );
    return out;
}

sys::bstream&
ggg::operator<<(sys::bstream& out, const group& rhs) {
    return out << rhs._name
               << rhs._gid
               << rhs._members;
}

sys::bstream&
ggg::operator>>(sys::bstream& in, group& rhs) {
    return in
           >> rhs._name
           >> rhs._gid
           >> rhs._members;
}

void
ggg::operator>>(const sqlite::statement& in, group& rhs) {
    rhs._name.clear();
    rhs._gid = -1;
    sqlite::cstream cstr(in);
    cstr >> rhs._gid >> rhs._name;
}
