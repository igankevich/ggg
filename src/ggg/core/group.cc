#include "group.hh"

#include <cstring>
#include <memory>
#include <unistdx/it/intersperse_iterator>

namespace ggg {

	template <class Container, class Ch>
	sys::basic_bstream<Ch>&
	operator<<(sys::basic_bstream<Ch>& out, const Container& rhs) {
		out << uint32_t(rhs.size());
		for (const auto& elem : rhs) {
			out << elem;
		}
		return out;
	}

	template <class Ch, class T>
	sys::basic_bstream<Ch>&
	operator>>(sys::basic_bstream<Ch>& in, std::unordered_set<T>& rhs) {
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

template <class Ch>
std::basic_ostream<Ch>&
ggg::operator<<(std::basic_ostream<Ch>& out, const basic_group<Ch>& rhs) {
	typedef typename basic_group<Ch>::string_type string_type;
	out << rhs.name() << "::" << rhs.id() << ':';
	std::copy(
		rhs._members.begin(),
		rhs._members.end(),
		sys::intersperse_iterator<string_type,Ch,Ch>(out, Ch(','))
	);
	return out;
}

template <class Ch>
sys::basic_bstream<Ch>&
ggg::operator<<(sys::basic_bstream<Ch>& out, const basic_group<Ch>& rhs) {
	return out << rhs._name
	           << rhs._gid
	           << rhs._members;
}

template <class Ch>
sys::basic_bstream<Ch>&
ggg::operator>>(sys::basic_bstream<Ch>& in, basic_group<Ch>& rhs) {
	return in
	       >> rhs._name
	       >> rhs._gid
	       >> rhs._members;
}

sqlite::rstream&
ggg::operator>>(sqlite::rstream& in, basic_group<char>& rhs) {
	rhs._name.clear();
	rhs._gid = -1;
	sqlite::cstream cstr(in);
	if (in >> cstr) {
		cstr >> rhs._gid >> rhs._name;
	}
	return in;
}

template class ggg::basic_group<char>;

template std::basic_ostream<char>&
ggg::operator<<(std::basic_ostream<char>& out, const basic_group<char>& rhs);

template sys::basic_bstream<char>&
ggg::operator<<(sys::basic_bstream<char>& out, const basic_group<char>& rhs);

template sys::basic_bstream<char>&
ggg::operator>>(sys::basic_bstream<char>& in, basic_group<char>& rhs);
