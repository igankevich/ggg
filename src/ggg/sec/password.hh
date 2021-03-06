#ifndef GGG_SEC_PASSWORD_HH
#define GGG_SEC_PASSWORD_HH

#include <iosfwd>
#include <string>

#include <zxcvbn/zxcvbn.h>

#include <ggg/sec/secure_string.hh>
#include <ggg/config.hh>

namespace ggg {

    std::ostream&
    operator<<(std::ostream& out, ZxcTypeMatch_t rhs);

    class password_match {

    private:
        typedef ZxcMatch_t match_type;

    private:
        match_type* _match = nullptr;
        double _entropy = 0.0;

    public:
        password_match() = default;

        inline
        ~password_match() {
            if (this->_match) {
                ::ZxcvbnFreeInfo(this->_match);
            }
        }

        inline
        password_match(password_match&& rhs):
        _match(rhs._match),
        _entropy(rhs._entropy) {
            rhs._match = nullptr;
        }

        password_match(const password_match&) = delete;

        password_match&
        operator=(const password_match&) = delete;

        inline password_match&
        find(const char* password, const char** dict=nullptr) {
            this->_entropy = ZxcvbnMatch(password, dict, &this->_match);
            return *this;
        }

        inline explicit
        operator bool() const noexcept {
            return this->_match;
        }

        inline bool
        operator!() const noexcept {
            return !this->operator bool();
        }

        inline double
        entropy() const noexcept {
            return this->_entropy;
        }

        friend std::ostream&
        operator<<(std::ostream& out, const password_match& rhs);

    };

    std::ostream&
    operator<<(std::ostream& out, const password_match& rhs);

    void validate_password(const secure_string& new_password, double min_entropy);
    bool verify_password(const std::string& hashed_password, const char* password);

    inline bool
    verify_password(
        const std::string& hashed_password,
        const secure_string& password
    ) {
        return verify_password(hashed_password, password.data());
    }

}

#endif // vim:filetype=cpp
