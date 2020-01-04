#ifndef GGG_PROTO_BYTE_BUFFER_HH
#define GGG_PROTO_BYTE_BUFFER_HH

#include <string>

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
        inline void flip() { this->_limit = position(); this->_position = 0; }

        auto write(const_pointer src, size_type n) -> size_type;
        auto read(pointer dst, size_type n) -> size_type;
        void peek(pointer dst, size_type n);
        void bump(size_type n);
        void compact();
        void clear();

        template <class Sink> auto flush(Sink& dst) -> size_type {
            size_type nwritten = 0, n = 0;
            while (remaining() != 0 && (n = dst.write(data()+position(), remaining())) > 0) {
                this->_position += n;
                nwritten += n;
            }
            return nwritten;
        }

        template <class Source> auto fill(Source& src) -> size_type {
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
            this->read(&x, sizeof(T));
        }

        template <class C, class T, class A>
        void write(const std::basic_string<C,T,A>& x) {
            this->write(static_cast<sys::u32>(x.size()));
            this->write(x.data(), x.size());
        }

        template <class C, class T, class A>
        void read(std::basic_string<C,T,A>& x) {
            sys::u32 size = 0;
            this->read<sys::u32>(size);
            x.resize(size);
            this->read(&x[0], x.size());
        }

    };

}

#endif // vim:filetype=cpp
