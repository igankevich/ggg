#ifndef GGG_BITS_BUFCOPY_HH
#define GGG_BITS_BUFCOPY_HH

#include <cstring>
#include <string>

namespace ggg {

	namespace bits {

		union pointer {
			char* ptr;
			char bytes[sizeof(ptr)];
		};

		static_assert(sizeof(pointer) == sizeof(char*), "bad pointer size");
		static_assert(sizeof(pointer) == alignof(pointer), "bad pointer size");

		inline char*
		align_buffer(char* buffer) {
			const size_t remainder = size_t(buffer) % alignof(pointer);
			const size_t offset = remainder == 0 ? 0 : (alignof(pointer) - remainder);
			return buffer + offset;
		}

		inline char*
		bufcopy(char** field, char* dest, const char* src) {
			*field = dest;
			while ((*dest++ = *src++));
			return dest;
		}

		inline char*
		bufcopy(char** field, char* dest, const char* src, size_t n) {
			*field = dest;
			std::memcpy(dest, src, n);
			return dest + n;
		}

		inline char*
		bufcopy(char*** field, char* dest, const pointer src[], size_t n) {
			dest = align_buffer(dest);
			*field = reinterpret_cast<char**>(dest);
			size_t nbytes = n*sizeof(pointer);
			std::memcpy(dest, src, nbytes);
			return dest + nbytes;
		}

		union Vector {
			const void* ptr = nullptr;
			char bytes[sizeof(ptr)];
		};

		class Buffer {

		private:
			char* _buffer;

		public:

			inline explicit
			Buffer(char* buf): _buffer(buf) {}

			inline char*
			ptr() {
				return this->_buffer;
			}

			inline char**
			array_ptr() {
				return reinterpret_cast<char**>(this->_buffer);
			}

			inline void
			align() {
				size_t remainder = size_t(this->_buffer) % alignof(pointer);
				size_t offset = remainder == 0 ? 0 : (alignof(pointer) - remainder);
				this->_buffer += offset;
			}

			inline char*
			write(const char* src) {
				char* old = this->_buffer;
				while ((*this->_buffer++ = *src++));
				return old;
			}

			inline char*
			write(const void* src, size_t n) {
				char* old = this->_buffer;
				std::memcpy(this->_buffer, src, n);
				this->_buffer += n;
				return old;
			}

			inline char*
			write(const std::string& s) {
				return this->write(s.data());
			}

			inline char**
			write(const Vector array[], size_t n) {
				this->align();
				char** old = this->array_ptr();
				for (size_t i=0; i<n; ++i) {
					this->write(array[i].bytes, sizeof(array[i]));
				}
				Vector last;
				this->write(last.bytes, sizeof(Vector));
				return old;
			}

		};

	}

}

#endif // vim:filetype=cpp
