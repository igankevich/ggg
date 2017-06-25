#ifndef ACCOUNT_HH
#define ACCOUNT_HH

#include <shadow.h>
#include <string>
#include <chrono>
#include <ostream>
#include <istream>

namespace ggg {

	class account {

	public:
		typedef std::string string;
		typedef std::chrono::system_clock clock_type;
		typedef clock_type::time_point time_point;
		typedef clock_type::duration duration;

	private:
		string _login;
		string _password;
		time_point _lastchange = time_point(duration::zero());
		duration _minchange = duration::zero();
		duration _maxchange = duration::zero();
		duration _warnchange = duration::zero();
		duration _maxinactive = duration::zero();
		time_point _expire = time_point(duration::zero());

	public:
		account() = default;
		account(const account&) = default;
		account(account&&) = default;
		account& operator=(const account&) = default;

		inline void
		make_expired() noexcept {
			this->_expire = clock_type::now();
		}

		inline bool
		is_expired() const noexcept {
			return this->_expire < clock_type::now();
		}

		size_t
		buffer_size() const noexcept;

		void
		copy_to(struct ::spwd* lhs, char* buffer) const;

		friend std::ostream&
		operator<<(std::ostream& out, const account& rhs);

		friend std::istream&
		operator>>(std::istream& in, account& rhs);

		inline const string&
		login() const noexcept {
			return this->_login;
		}

		inline const string&
		password() const noexcept {
			return this->_password;
		}

		string
		password_id() const;

		inline string
		password_salt() const {
			return password_salt(false);
		}

		inline string
		password_prefix() const {
			return password_salt(true);
		}

		inline time_point
		last_change() const noexcept {
			return this->_lastchange;
		}

		inline duration
		min_change() const noexcept {
			return this->_minchange;
		}

		inline duration
		max_change() const noexcept {
			return this->_maxchange;
		}

		inline duration
		warn_change() const noexcept {
			return this->_warnchange;
		}

		inline duration
		max_inactive() const noexcept {
			return this->_maxinactive;
		}

		inline time_point
		expire() const noexcept {
			return this->_expire;
		}

	private:
		string
		password_salt(bool with_id) const;
	};

	std::ostream&
	operator<<(std::ostream& out, const account& rhs);

	std::istream&
	operator>>(std::istream& in, account& rhs);
}

#endif // ACCOUNT_HH
