#ifndef CTL_PASSWORD_HH
#define CTL_PASSWORD_HH

#include "sec/secure_string.hh"

namespace ggg {

	secure_string
	generate_salt();

	secure_string
	encrypt(const char* password, const secure_string& prefix);

}

#endif // CTL_PASSWORD_HH vim:filetype=cpp
