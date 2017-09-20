#include "password.hh"

#include <config.hh>
#include <core/account.hh>
#include <crypt.h>
#include <random>

namespace {
	typedef std::independent_bits_engine<std::random_device, 8, unsigned char>
		engine_type;
}

ggg::secure_string
ggg::generate_salt() {
	secure_string salt;
	engine_type engine;
	int i = 0;
	while (i < GGG_SALT_LENGTH) {
		const char ch = engine();
		if (std::isgraph(ch)
			&& ch != ggg::account::separator
			&& ch != ggg::account::delimiter)
		{
			salt.push_back(ch);
			++i;
		}
	}
	return salt;
}

ggg::secure_string
ggg::encrypt(const char* password, const secure_string& prefix) {
	secure_allocator<crypt_data> alloc;
	std::unique_ptr<crypt_data> pdata(alloc.allocate(1));
	char* encrypted = ::crypt_r(
		password,
		prefix.data(),
		pdata.get()
	);
	if (!encrypted) {
		throw std::system_error(errno, std::system_category());
	}
	secure_string result(encrypted);
	size_t n = account::string::traits_type::length(encrypted);
	shred(encrypted, n);
	return result;
}

