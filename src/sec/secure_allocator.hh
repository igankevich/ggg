#ifndef SECURE_ALLOCATOR_HH
#define SECURE_ALLOCATOR_HH

#include <memory>
#include <cstring>
#include <system_error>
#include <cstdlib>

#include <unistd.h>
#if defined(_POSIX_MEMLOCK_RANGE) || defined(_POSIX_MEMORY_PROTECTION)
	#include <sys/mman.h>
#endif

#include "shred.hh"

namespace ggg {

	template<class T>
	class secure_allocator: public std::allocator<T> {

		typedef void* void_ptr;

	public:
		typedef std::allocator<T> base_allocator;
		using typename base_allocator::value_type;
		using typename base_allocator::pointer;
		using typename base_allocator::const_pointer;
		using typename base_allocator::size_type;

		secure_allocator() = default;
		secure_allocator(const secure_allocator&) = default;
		template <class U>
		secure_allocator(const secure_allocator<U>& rhs) {}

		pointer
		allocate(size_type n, const_pointer hint=0) {
			n *= sizeof(T);
			pointer p = nullptr;
		   	if (-1 == ::posix_memalign((void**)&p, page_size, n)) {
				throw std::bad_alloc();
			}
			lock_and_protect(p, n);
			shred(p, n);
			return p;
		}

		void
		deallocate(pointer p, size_type n) {
			n *= sizeof(T);
			shred(p, n);
			::free(p);
		}

		template <class U>
		struct rebind {
			typedef secure_allocator<U> other;
		};

	private:
		void
		lock_and_protect(void_ptr p, size_type n) {
			#if defined(_POSIX_MEMLOCK_RANGE)
			call("mlock", ::mlock(p, n));
			#endif
			#if defined(_POSIX_MEMORY_PROTECTION)
			call("mprotect", ::mprotect(p, n, PROT_READ|PROT_WRITE));
			#endif
		}

		void
		call(const char* func, int ret) {
			if (ret == -1) {
				throw std::system_error(errno, std::system_category());
			}
		}

		static const size_t page_size;
	};

	template <class T>
	const size_t secure_allocator<T>::page_size = ::sysconf(_SC_PAGESIZE);

}

#endif // SECURE_ALLOCATOR_HH
