#ifndef GGG_BITS_MACROS_HH
#define GGG_BITS_MACROS_HH

#include <ggg/config.hh>
#if defined(GGG_HAVE_VISIBILITY)
#define GGG_VISIBILITY_DEFAULT [[gnu::visibility("default")]]
#else
#define GGG_VISIBILITY_DEFAULT
#endif

#endif // vim:filetype=cpp
