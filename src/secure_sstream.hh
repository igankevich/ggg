#ifndef SECURE_SSTREAM_HH
#define SECURE_SSTREAM_HH

#include <sstream>
#include "secure_allocator.hh"

namespace ggg {

	typedef std::basic_stringstream<
		char,
		std::char_traits<char>,
		secure_allocator<char>
	> secure_stringstream;

}

#endif // SECURE_SSTREAM_HH

