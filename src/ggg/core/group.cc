#include "group.hh"

#include <ggg/bits/bufcopy.hh>

#include <cstring>
#include <memory>
#include <unistdx/it/intersperse_iterator>

namespace {
	union pointer {
		char* ptr;
		char bytes[sizeof(ptr)];
	};
	static_assert(sizeof(pointer) == sizeof(char*), "bad pointer size");
	static_assert(sizeof(pointer) == alignof(pointer), "bad pointer size");
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
	const size_t remainder = size_t(buffer) % alignof(pointer);
	const size_t offset = remainder == 0 ? 0 : (alignof(pointer) - remainder);
	buffer += offset;
	// store address of the first basic_group<Ch> member in the gr_mem field
	lhs->gr_mem = reinterpret_cast<char**>(buffer);
	// copy pointers to basic_group<Ch> members as well
	std::memcpy(buffer, mem.get(), sizeof(pointer) * nmem);
	buffer += sizeof(pointer) * nmem;
	lhs->gr_gid = gr.id();
}

template class ggg::basic_group<char>;
template class ggg::basic_group<wchar_t>;

template std::basic_ostream<char>&
ggg::operator<<(std::basic_ostream<char>& out, const basic_group<char>& rhs);

template std::basic_ostream<wchar_t>&
ggg::operator<<(
	std::basic_ostream<wchar_t>& out,
	const basic_group<wchar_t>& rhs
);

template size_t
ggg::buffer_size(const basic_group<char>&);

template void
ggg::copy_to(const basic_group<char>&, struct ::group*, char*);
