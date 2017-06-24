#include "group.hh"
#include "bufcopy.hh"
#include <stdx/iterator.hh>
#include <cstring>
#include <memory>

namespace {
	union pointer {
		char* ptr;
		char bytes[sizeof(ptr)];
	};
	static_assert(sizeof(pointer) == sizeof(char*), "bad pointer size");
	static_assert(sizeof(pointer) == alignof(pointer), "bad pointer size");
}

std::ostream&
ggg::operator<<(std::ostream& out, const group& rhs) {
	out << rhs.name() << ':'
		<< rhs.password() << ':'
		<< rhs.id() << ':';
	std::copy(
		rhs._members.begin(),
		rhs._members.end(),
		stdx::intersperse_iterator<std::string>(out, ",")
	);
	return out;
}

size_t
ggg::group::buffer_size() const noexcept {
	size_t sum = 0;
	sum += this->_name.size();
	sum += this->_password.size();
	for (const std::string& member : this->_members) {
		sum += member.size() + 1 + sizeof(pointer);
	}
	sum += alignof(pointer) - 1;
	return sum;
}

void
ggg::group::copy_to(struct ::group* lhs, char* buffer) const {
	buffer = bits::bufcopy(&lhs->gr_name, buffer, this->_name.data());
	buffer = bits::bufcopy(&lhs->gr_passwd, buffer, this->_password.data());
	// copy each group member
	const size_t nmem = _members.size() + 1;
	std::unique_ptr<pointer[]> mem(new pointer[nmem]);
	size_t i = 0;
	for (const std::string& member : _members) {
		buffer = bits::bufcopy(&mem[i].ptr, buffer, member.data());
		++i;
	}
	mem[i].ptr = nullptr;
	// align the buffer
	const size_t remainder = size_t(buffer) % alignof(pointer);
	const size_t offset = remainder == 0 ? 0 : (alignof(pointer) - remainder);
	buffer += offset;
	// store address of the first group member in the gr_mem field
	lhs->gr_mem = reinterpret_cast<char**>(buffer);
	// copy pointers to group members as well
	std::memcpy(buffer, mem.get(), sizeof(pointer) * nmem);
	buffer += sizeof(pointer) * nmem;
	lhs->gr_gid = this->_gid;
}
