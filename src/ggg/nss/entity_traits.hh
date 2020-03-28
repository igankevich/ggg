#ifndef GGG_NSS_ENTITY_TRAITS_HH
#define GGG_NSS_ENTITY_TRAITS_HH

#include <cstddef>

namespace ggg {

    template <class T>
    struct entity_traits;

    template <class T>
    size_t
    buffer_size(const T& ent) noexcept;

    template <class X, class Y>
    void
    copy_to(const X& a, Y* lhs, char* buffer);

}

#endif // vim:filetype=cpp
