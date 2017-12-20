#ifndef ACCOUNT_HH
#define ACCOUNT_HH

#include <shadow.h>
#include <chrono>
#include <ostream>
#include <istream>
#include <type_traits>
#include "sec/secure_string.hh"
#include "account_flags.hh"
#include "form_field.hh"

namespace ggg {

	namespace chrono {

		typedef std::chrono::duration<long,std::ratio<60*60*24,1>> days;

	}

	class account {

	public:
		typedef secure_string string;
		typedef std::chrono::system_clock clock_type;
		typedef clock_type::time_point time_point;
		typedef clock_type::duration duration;
		typedef const size_t columns_type[9];

		static constexpr const char delimiter = ':';
		static constexpr const char separator = '$';

	private:
		string _login;
		string _id;
		string _salt;
		unsigned int _nrounds = 0;
		string _password;
		time_point _lastchange = time_point(duration::zero());
		duration _minchange = duration::zero();
		duration _maxchange = duration::zero();
		duration _warnchange = duration::zero();
		duration _maxinactive = duration::zero();
		time_point _expire = time_point(duration::zero());
		account_flags _flags = account_flags(0);

	public:
		inline explicit
		account(const char* login):
		_login(login)
		{}

		account() = default;
		account(const account&) = default;
		account(account&&) = default;
		account& operator=(const account&) = default;

		inline void
		make_expired() noexcept {
			this->_expire = clock_type::now() - duration(duration::rep(1));
		}

		inline void
		make_active() noexcept {
			this->_expire = time_point(duration::zero());
		}

		inline bool
		has_expiration_date() const noexcept {
			return this->_expire > time_point(duration::zero());
		}

		inline bool
		has_expired() const noexcept {
			return this->has_expired(clock_type::now());
		}

		inline bool
		has_expired(time_point now) const noexcept {
			return this->has_expiration_date() && this->_expire < now;
		}

		inline bool
		password_has_expired() const noexcept {
			return this->password_has_expired(clock_type::now());
		}

		inline bool
		password_has_expired(time_point now) const noexcept {
			return (this->_lastchange > time_point(duration::zero())
				&& this->_maxchange > duration::zero()
				&& this->_lastchange < now - this->_maxchange)
				|| (this->_flags & account_flags::password_has_expired);
		}

		inline void
		reset_password() noexcept {
			this->setf(account_flags::password_has_expired);
		}

		size_t
		buffer_size() const noexcept;

		void
		copy_to(struct ::spwd* lhs, char* buffer) const;

		friend std::ostream&
		operator<<(std::ostream& out, const account& rhs);

		friend std::istream&
		operator>>(std::istream& in, account& rhs);

		void
		write_human(std::ostream& out, columns_type width) const;

		std::istream&
		read_human(std::istream& in);

		void
		copy_from(const account& rhs);

		inline bool
		operator==(const account& rhs) const noexcept {
			return this->_login == rhs._login;
		}

		inline bool
		operator!=(const account& rhs) const noexcept {
			return !operator==(rhs);
		}

		inline bool
		operator<(const account& rhs) const noexcept {
			return this->_login < rhs._login;
		}

		inline const string&
		login() const noexcept {
			return this->_login;
		}

		inline const string&
		password() const noexcept {
			return this->_password;
		}

		inline const string&
		password_id() const noexcept {
			return this->_id;
		}

		inline unsigned int
		num_rounds() const noexcept {
			return this->_nrounds;
		}

		inline const string&
		password_salt() const noexcept {
			return this->_salt;
		}

		string
		password_prefix() const;

		static string
		password_prefix(
			const string& new_salt,
			const string& new_id,
			unsigned int nrounds
		);

		void
		set_password(const string& rhs);

		inline bool
		has_password() const noexcept {
			return !this->_password.empty();
		}

		inline time_point
		last_change() const noexcept {
			return this->_lastchange;
		}

		inline bool
		has_last_change() const noexcept {
			return this->_lastchange > time_point(duration::zero());
		}

		inline duration
		min_change() const noexcept {
			return this->_minchange;
		}

		inline bool
		has_min_change() const noexcept {
			return this->_minchange > duration::zero();
		}

		inline duration
		max_change() const noexcept {
			return this->_maxchange;
		}

		inline bool
		has_max_change() const noexcept {
			return this->_maxchange > duration::zero();
		}

		inline duration
		warn_change() const noexcept {
			return this->_warnchange;
		}

		inline bool
		has_warn_change() const noexcept {
			return this->_warnchange > duration::zero();
		}

		inline duration
		max_inactive() const noexcept {
			return this->_maxinactive;
		}

		inline bool
		has_max_inactive() const noexcept {
			return this->_maxinactive > duration::zero();
		}

		inline time_point
		expire() const noexcept {
			return this->_expire;
		}

		inline account_flags
		flags() const noexcept {
			return this->_flags;
		}

		inline void
		setf(account_flags rhs) noexcept {
			this->_flags = this->_flags | rhs;
		}

		inline void
		unsetf(account_flags rhs) noexcept {
			this->_flags = this->_flags & (~rhs);
		}

		inline void
		clear(account_flags rhs) noexcept {
			this->_flags = rhs;
		}

		inline bool
		is_recruiter() const noexcept {
			return this->_flags & account_flags::recruiter;
		}

		inline void
		set_max_change(duration rhs) noexcept {
			this->_maxchange = rhs;
		}

		void clear();

		void
		set(const form_field& field, const char* value);

	private:
		void
		parse_password();

	};

	std::ostream&
	operator<<(std::ostream& out, const account& rhs);

	std::istream&
	operator>>(std::istream& in, account& rhs);
}

#endif // ACCOUNT_HH
