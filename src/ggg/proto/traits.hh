#ifndef GGG_PROTO_TRAITS_HH
#define GGG_PROTO_TRAITS_HH

#include <unistdx/base/byte_buffer>

namespace ggg {

    template <class T>
    struct Protocol_traits {
        static void write(sys::byte_buffer& buf, const T& rhs);
        static void read(sys::byte_buffer& buf, T& rhs);
    };

}

#endif // vim:filetype=cpp
