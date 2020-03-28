#ifndef GGG_NSS_BUFFER_HH
#define GGG_NSS_BUFFER_HH

#include <cstring>
#include <string>

namespace ggg {

    union Pointer {
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
            size_t remainder = size_t(this->_buffer) % alignof(Pointer);
            size_t offset = remainder == 0 ? 0 : (alignof(Pointer) - remainder);
            this->_buffer += offset;
        }

        inline char*
        write(char ch) {
            char* old = this->_buffer;
            *this->_buffer++ = ch;
            return old;
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
        write(const Pointer array[], size_t n) {
            this->align();
            char** old = this->array_ptr();
            for (size_t i=0; i<n; ++i) {
                this->write(array[i].bytes, sizeof(array[i]));
            }
            Pointer last;
            this->write(last.bytes, sizeof(Pointer));
            return old;
        }

    };

}

#endif // vim:filetype=cpp
