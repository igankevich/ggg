#include "password.hh"

#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/account.hh>
#include <ggg/ggg_crypt.hh>
#include <ggg/sec/argon2.hh>
#include <ggg/sec/random.hh>

namespace {

	using engine_type =
        std::independent_bits_engine<ggg::unpredictable_random_engine,
        8, unsigned char>;

    std::string
    algorithm(const std::string& hashed_password) {
        if (hashed_password.empty() || hashed_password.front() != '$') { return "6"; }
        auto first = hashed_password.begin();
        auto last = hashed_password.end();
        auto from = ++first;
        std::string id("6");
        while (first != last) {
            if (*first == '$') {
                id.assign(from, first);
                break;
            }
            ++first;
        }
        return id;
    }

}

std::string
ggg::generate_salt(size_t length) {
	std::string salt;
	engine_type engine;
	size_t i = 0;
	while (i < length) {
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

std::string
ggg::sha512_password_hash::hash(const secure_string& password) const {
	return password_hash_sha512(password.data(), this->prefix(this->salt())).data();
}

std::string
ggg::sha512_password_hash::prefix(const std::string& salt) const {
	std::stringstream s;
	s.put('$');
	s.put('6');
	s.put('$');
	if (this->_nrounds > 0) {
		s << "rounds=" << this->_nrounds;
		s.put('$');
	}
	s << salt;
	s.put('$');
	return s.str();
}

void ggg::sha512_password_hash::parse_hashed_password(const char* hashed_password) {
	using traits_type = std::string::traits_type;
    auto length = traits_type::length(hashed_password);
	if (length == 0 || *hashed_password != '$') {
		return;
	}
	const char rounds[] = "rounds=";
	const ptrdiff_t rounds_size = sizeof(rounds) - 1;
	const char* first = hashed_password + 1;
	const char* last = hashed_password + length;
	const char* prev = first;
	size_t field_no = 0;
    std::string id;
	while (first != last) {
		if (*first == '$') {
			if (field_no == 0) {
				id.assign(prev, first);
			} else if (0 == traits_type::compare(
				rounds,
				prev,
				std::min(rounds_size, first-prev)
			)) {
                std::stringstream str(std::string(prev+rounds_size, first));
				str >> this->_nrounds;
			} else {
				this->_salt.assign(prev, first);
			}
			++field_no;
			prev = first;
			++prev;
		}
		++first;
	}
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
ggg::validate_password(const secure_string& new_password, double min_entropy) {
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

bool
ggg::verify_password(const std::string& hashed_password, const char* password) {
    auto algo = algorithm(hashed_password);
    bool success = false;
    if (algo == "6") {
        success = ggg::sha512_password_hash::verify(hashed_password, password);
    } else if (algo == "argon2id") {
        ggg::init_sodium();
        success = ggg::argon2_password_hash::verify(hashed_password, password);
    }
    return success;
}

