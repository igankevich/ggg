#include "password.hh"

#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/ggg_crypt.hh>

namespace {
	typedef std::independent_bits_engine<std::random_device, 8, unsigned char>
		engine_type;
}

std::string
ggg::generate_salt() {
	std::string salt;
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
ggg::encrypt(const char* password, const std::string& prefix) {
	return password_hash_sha512(password, prefix).data();
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

void
ggg::validate_password(secure_string new_password, double min_entropy) {
    if (new_password.empty()) { throw std::invalid_argument("Empty password"); }
	// TODO consider user name
	ggg::password_match match;
	if (match.find(new_password.data()) &&
		match.entropy() < min_entropy) {
		std::stringstream msg;
		msg << "Weak password. Password strength is "
			<< int(match.entropy() / min_entropy * 100.0)
			<< "/100.";
		throw std::invalid_argument(msg.str());
	}
}

