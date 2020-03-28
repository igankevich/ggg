#include "password.hh"

#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/account.hh>
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
    if (algo == "argon2id") {
        ggg::init_sodium();
        success = ggg::argon2_password_hash::verify(hashed_password, password);
    }
    return success;
}

