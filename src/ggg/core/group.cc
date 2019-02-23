#include "group.hh"

#include <ggg/bits/bufcopy.hh>

#include <cstring>
#include <memory>
#include <unistdx/it/intersperse_iterator>

namespace {

	using ggg::bits::pointer;

}

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
	out << rhs.name() << ':'
	    << rhs.password() << ':'
	    << rhs.id() << ':';
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
	           << rhs._password
	           << rhs._gid
	           << rhs._members;
}

template <class Ch>
sys::basic_bstream<Ch>&
ggg::operator>>(sys::basic_bstream<Ch>& in, basic_group<Ch>& rhs) {
	return in
	       >> rhs._name
	       >> rhs._password
	       >> rhs._gid
	       >> rhs._members;
}

template <class Ch>
size_t
ggg
::buffer_size(const basic_group<Ch>& gr) noexcept {
	size_t sum = 0;
	sum += gr.name().size();
	sum += gr.password().size();
	for (const std::string& member : gr.members()) {
		sum += member.size() + 1 + sizeof(pointer);
	}
	sum += alignof(pointer) - 1;
	return sum;
}

template <class Ch>
void
ggg
::copy_to(const basic_group<Ch>& gr, struct ::group* lhs, char* buffer) {
	buffer = bits::bufcopy(&lhs->gr_name, buffer, gr.name().data());
	buffer = bits::bufcopy(&lhs->gr_passwd, buffer, gr.password().data());
	// copy each basic_group<Ch> member
	const size_t nmem = gr.members().size() + 1;
	std::unique_ptr<pointer[]> mem(new pointer[nmem]);
	size_t i = 0;
	for (const std::string& member : gr.members()) {
		buffer = bits::bufcopy(&mem[i].ptr, buffer, member.data());
		++i;
	}
	mem[i].ptr = nullptr;
	// align the buffer
	buffer = bits::align_buffer(buffer);
	// store address of the first basic_group<Ch> member in the gr_mem field
	lhs->gr_mem = reinterpret_cast<char**>(buffer);
	// copy pointers to basic_group<Ch> members as well
	std::memcpy(buffer, mem.get(), sizeof(pointer) * nmem);
	buffer += sizeof(pointer) * nmem;
	lhs->gr_gid = gr.id();
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

template size_t
ggg
::buffer_size(const basic_group<char>&);

template void
ggg
::copy_to(const basic_group<char>&, struct ::group*, char*);

template sys::basic_bstream<char>&
ggg::operator<<(sys::basic_bstream<char>& out, const basic_group<char>& rhs);

template sys::basic_bstream<char>&
ggg::operator>>(sys::basic_bstream<char>& in, basic_group<char>& rhs);
