#include "password.hh"

#include <crypt.h>

#include <ostream>
#include <random>

#include <config.hh>
#include <core/account.hh>

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

std::ostream&
ggg::operator<<(std::ostream& out, ZxcTypeMatch_t rhs) {
	const char* s = "<unknown>";
	switch (rhs) {
	case NON_MATCH: s = "NON_MATCH"; break;
	case BRUTE_MATCH: s = "BRUTE_MATCH"; break;
	case DICTIONARY_MATCH: s = "DICTIONARY_MATCH"; break;
	case DICT_LEET_MATCH: s = "DICT_LEET_MATCH"; break;
	case USER_MATCH: s = "USER_MATCH"; break;
	case USER_LEET_MATCH: s = "USER_LEET_MATCH"; break;
	case REPEATS_MATCH: s = "REPEATS_MATCH"; break;
	case SEQUENCE_MATCH: s = "SEQUENCE_MATCH"; break;
	case SPATIAL_MATCH: s = "SPATIAL_MATCH"; break;
	case DATE_MATCH: s = "DATE_MATCH"; break;
	case YEAR_MATCH: s = "YEAR_MATCH"; break;
	case MULTIPLE_MATCH: s = "MULTIPLE_MATCH"; break;
	default: break;
	}
	return out << s;
}

std::ostream&
ggg::operator<<(std::ostream& out, const password_match& rhs) {
	out << "entropy=" << rhs._entropy << '\n';
	password_match::match_type* next = rhs._match;
	while (next) {
		out << "begin=" << next->Begin
			<< ",length=" << next->Length
			<< ",entropy=" << next->Entrpy
			<< ",multipart-entropy=" << next->MltEnpy
			<< ",type=" << next->Type
			<< '\n';
		next = next->Next;
	}
	return out;
}

