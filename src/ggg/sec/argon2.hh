#ifndef GGG_SEC_ARGON2_HH
#define GGG_SEC_ARGON2_HH

#include <ggg/sec/secure_string.hh>

namespace ggg {

    class argon2_password_hash {

    private:
        unsigned long long _opslimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
        unsigned long long _memlimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;

    public:

        inline std::string
        operator()(const secure_string& password) {
            return this->hash(password);
        }

        inline void memory_limit(unsigned long long rhs) { this->_memlimit = rhs; }
        inline void time_limit(unsigned long long rhs) { this->_opslimit = rhs; }
        inline unsigned long long memory_limit() const { return this->_memlimit; }
        inline unsigned long long time_limit() const { return this->_opslimit; }

        inline std::string
        hash(const secure_string& password) const {
            std::string hashed_password(crypto_pwhash_STRBYTES, 0);
            int ret = ::crypto_pwhash_str(&hashed_password[0],
                    password.data(), password.size(),
                    this->_opslimit, this->_memlimit);
            if (ret != 0) { throw std::bad_alloc(); }
            using traits_type = secure_string::traits_type;
            hashed_password.resize(traits_type::length(hashed_password.data()));
            return hashed_password;
        }

        static inline bool
        verify(const std::string& hashed_password, const secure_string& password) {
            return ::crypto_pwhash_str_verify(hashed_password.data(),
                    password.data(), password.size()) == 0;
        }

        static inline bool
        verify(const char* hashed_password, const secure_string& password) {
            return ::crypto_pwhash_str_verify(hashed_password,
                    password.data(), password.size()) == 0;
        }

    };

}

#endif // vim:filetype=cpp
