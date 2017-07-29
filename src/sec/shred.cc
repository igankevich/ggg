#include "shred.hh"
#include <cstring>

void
ggg::shred(void* p, size_t n) noexcept {
	std::memset(p, 11, n);
	std::memset(p, 22, n);
	std::memset(p, 33, n);
	std::memset(p, 44, n);
}
