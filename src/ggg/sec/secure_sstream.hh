#ifndef GGG_SEC_SECURE_SSTREAM_HH
#define GGG_SEC_SECURE_SSTREAM_HH

#include <sstream>
#include "secure_allocator.hh"

namespace ggg {

	typedef std::basic_stringstream<
		char,
		std::char_traits<char>,
		secure_allocator<char>
	> secure_stringstream;

}

#endif // vim:filetype=cpp

