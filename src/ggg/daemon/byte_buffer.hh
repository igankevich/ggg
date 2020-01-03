#ifndef GGG_DAEMON_BYTE_BUFFER_HH
#define GGG_DAEMON_BYTE_BUFFER_HH

#include <unistdx/base/byte_buffer>
#include <unistdx/base/types>

namespace ggg {

    template <class T>
    constexpr bool is_basic() {
        return std::is_same<T,bool>::value ||
               std::is_same<T,char>::value ||
               std::is_same<T,unsigned char>::value ||
               std::is_same<T,char16_t>::value ||
               std::is_same<T,char32_t>::value ||
               std::is_same<T,sys::u8>::value ||
               std::is_same<T,sys::u16>::value ||
               std::is_same<T,sys::u32>::value ||
               std::is_same<T,sys::u64>::value ||
               std::is_same<T,sys::i8>::value ||
               std::is_same<T,sys::i16>::value ||
               std::is_same<T,sys::i32>::value ||
               std::is_same<T,sys::i64>::value ||
               std::is_same<T,sys::f32>::value ||
               std::is_same<T,sys::f64>::value
               #if defined(UNISTDX_HAVE_LONG_DOUBLE)
               || std::is_same<T,sys::f128>::value
               #endif
               ;
    }

    class byte_buffer: public sys::byte_buffer {

    public:
        using sys::byte_buffer::size_type;
        using sys::byte_buffer::value_type;
        using sys::byte_buffer::iterator;
        using sys::byte_buffer::const_iterator;
        using pointer = void*;
        using const_pointer = const void*;

    private:
        size_type _position = 0;
        size_type _limit = 0;

    public:
        using sys::byte_buffer::byte_buffer;

        byte_buffer() = default;

		inline explicit
		byte_buffer(size_type size): sys::byte_buffer(size), _limit(this->size()) {}

        inline size_type position() const { return this->_position; }
        inline size_type limit() const { return this->_limit; }
        inline size_type remaining() const { return this->_limit - this->_position; }
        inline void position(size_type rhs) { this->_position = rhs; }
        inline void limit(size_type rhs) { this->_limit = rhs; }
        inline void bump(size_type n) { this->_position += n; }

        auto write(const_pointer src, size_type n) -> size_type;
        auto read(pointer dst, size_type n) -> size_type;
        void flip();
        void compact();
        void clear();

        template <class Sink> auto write(Sink& dst) -> size_type {
            size_type nwritten = 0, n = 0;
            while ((n = dst.write(data()+position(), remaining())) > 0) {
                this->_position += n;
                nwritten += n;
            }
            return nwritten;
        }

        template <class Source> auto read(Source& src) -> size_type {
            size_type nread = 0, n = 0;
            while ((n = src.read(data()+position(), remaining())) > 0) {
                if (remaining() == 0) { grow(); this->_limit = size(); }
                this->_position += n;
                nread += n;
            }
            return nread;
        }

        template <class T>
        auto write(T x) -> typename std::enable_if<is_basic<T>(),void>::type {
            this->write(&x, sizeof(T));
        }

        template <class T>
        auto read(T& x) -> typename std::enable_if<is_basic<T>(),void>::type {
            this->write(&x, sizeof(T));
        }

    };

}

#endif // vim:filetype=cpp
