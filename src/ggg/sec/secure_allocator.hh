#ifndef GGG_SEC_SECURE_ALLOCATOR_HH
#define GGG_SEC_SECURE_ALLOCATOR_HH

#include <memory>
#include <stdexcept>

#include <sodium.h>

namespace ggg {

    inline void
    init_sodium() {
        if (::sodium_init() == -1) {
            throw std::runtime_error("failed to init libsodium");
        }
    }

    inline void shred(void* p, size_t n) { ::sodium_memzero(p, n); }

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
            init_sodium();
			pointer p = static_cast<pointer>(::sodium_malloc(n));
            if (!p) { throw std::bad_alloc(); }
			lock(p, n);
			return p;
		}

		void
		deallocate(pointer p, size_type n) {
			n *= sizeof(T);
			shred(p, n);
			unlock(p, n);
			free(p);
		}

		template <class U>
		struct rebind {
			typedef secure_allocator<U> other;
		};

	private:

        inline void free(pointer p) { ::sodium_free(p); }
        inline void lock(pointer p, size_type n) { ::sodium_mlock(p, n); }
        inline void unlock(pointer p, size_type n) { ::sodium_munlock(p, n); }

	};

}

#endif // vim:filetype=cpp
