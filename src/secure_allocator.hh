#ifndef SECURE_ALLOCATOR_HH
#define SECURE_ALLOCATOR_HH

#include <memory>
#include <cstring>
#include <system_error>

#include <unistd.h>
#if defined(_POSIX_MEMLOCK_RANGE) || defined(_POSIX_MEMORY_PROTECTION)
	#include <sys/mman.h>
#endif

namespace sys {

	void
	shred(void* p, size_t n) noexcept {
		std::memset(p, 11, n);
		std::memset(p, 22, n);
		std::memset(p, 33, n);
		std::memset(p, 44, n);
	}


	template<class T>
	class secure_allocator: public std::allocator<T> {

		typedef void* void_ptr;

	public:
		typedef std::allocator<T> base_allocator;
		using typename base_allocator::value_type;
		using typename base_allocator::pointer;
		using typename base_allocator::const_pointer;
		using typename base_allocator::size_type;

		pointer
		allocate(size_type n, const_pointer hint=0) {
			pointer p = base_allocator::allocate(n, hint);
			lock_and_protect(p, n);
			shred(p, n);
			return p;
		}

		void
		deallocate(pointer p, size_type n) {
			shred(p, n);
			base_allocator::deallocate(p, n);
		}

	private:
		void
		lock_and_protect(void_ptr p, size_type n) {
			#if defined(_POSIX_MEMLOCK_RANGE)
			call(::mlock(p, n));
			#endif
			#if defined(_POSIX_MEMORY_PROTECTION)
			call(::mprotect(p, n, PROT_READ|PROT_WRITE));
			#endif
		}

		void
		call(int ret) {
			if (ret == -1) {
				throw std::system_error(errno, std::system_category());
			}
		}

	};

}

#endif // SECURE_ALLOCATOR_HH
