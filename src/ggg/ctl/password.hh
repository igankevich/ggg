#ifndef CTL_PASSWORD_HH
#define CTL_PASSWORD_HH

#include <iosfwd>
#include <string>

#include <zxcvbn/zxcvbn.h>

#include <ggg/sec/secure_string.hh>
#include <ggg/config.hh>

namespace ggg {

	std::string generate_salt(size_t length);
	inline std::string generate_salt() { return generate_salt(GGG_SALT_LENGTH); }

    class sha512_password_hash {

    private:
        size_t _saltlength = GGG_SALT_LENGTH;
        size_t _nrounds = 0;
        std::string _salt;

    public:

        sha512_password_hash() = default;

        inline explicit
        sha512_password_hash(const secure_string& hashed_password):
        sha512_password_hash(hashed_password.data()) {}

        inline explicit
        sha512_password_hash(const std::string& hashed_password):
        sha512_password_hash(hashed_password.data()) {}

        inline explicit
        sha512_password_hash(const char* hashed_password) {
            this->parse_hashed_password(hashed_password);
        }

        inline std::string
        operator()(const secure_string& password) {
            return this->hash(password);
        }

        inline void salt_length(size_t rhs) { this->_saltlength = rhs; }
        inline size_t salt_length() const { return this->_saltlength; }
        inline void num_rounds(size_t rhs) { this->_nrounds = rhs; }
        inline size_t num_rounds() const { return this->_nrounds; }

        std::string hash(const secure_string& password) const;

        inline std::string
        salt() const {
            return this->_salt.empty()
                ? generate_salt(this->_saltlength)
                : this->_salt;
        }

        std::string prefix(const std::string& salt) const;

        static inline bool
        verify(const std::string& hashed_password, const secure_string& password) {
            sha512_password_hash hash(hashed_password);
            return hash(password) == hashed_password;
        }

        static inline bool
        verify(const char* hashed_password, const secure_string& password) {
            sha512_password_hash hash(hashed_password);
            return hash(password) == hashed_password;
        }

    private:

        void parse_hashed_password(const char* hashed_password);

    };

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

	void
	validate_password(const secure_string& new_password, double min_entropy);

}

#endif // CTL_PASSWORD_HH vim:filetype=cpp
