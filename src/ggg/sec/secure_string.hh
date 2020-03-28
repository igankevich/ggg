#ifndef GGG_SEC_SECURE_STRING_HH
#define GGG_SEC_SECURE_STRING_HH

#include <string>

#include <ggg/sec/secure_allocator.hh>

namespace ggg {

    typedef std::basic_string<
        char,
        std::char_traits<char>,
        secure_allocator<char>
    > secure_string;

}

#endif // vim:filetype=cpp
