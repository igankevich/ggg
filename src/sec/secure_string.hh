#ifndef SECURE_STRING_HH
#define SECURE_STRING_HH

#include <string>
#include "secure_allocator.hh"

namespace ggg {

	typedef std::basic_string<
		char,
		std::char_traits<char>,
		secure_allocator<char>
	> secure_string;

}

#endif // SECURE_STRING_HH
