#ifndef ACCOUNT_HH
#define ACCOUNT_HH

#include <shadow.h>

#include <chrono>
#include <istream>
#include <ostream>
#include <type_traits>

#include <unistdx/fs/path>

#include <ggg/core/account_flags.hh>
#include <ggg/core/entity_format.hh>
#include <ggg/core/eq_traits.hh>
#include <ggg/core/form_field.hh>
#include <ggg/sec/secure_string.hh>

#include <sqlitex/statement.hh>

namespace ggg {

	class account {

	public:
		typedef char char_type;
		typedef std::char_traits<char_type> traits_type;
		typedef std::basic_string<char_type, traits_type> string_type;
		typedef secure_string string;
		typedef std::chrono::system_clock clock_type;
		typedef clock_type::time_point time_point;
		typedef clock_type::duration duration;
		typedef const size_t columns_type[9];
		typedef sys::path path_type;

		static constexpr const char delimiter = ':';
		static constexpr const char separator = '$';

	private:
		string_type _login;
		string_type _id;
		string_type _salt;
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

		account&
		operator=(const account&) = default;

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

		friend std::ostream&
		operator<<(std::ostream& out, const account& rhs);

		friend std::istream&
		operator>>(std::istream& in, account& rhs);

		friend void
		operator>>(const sqlite::statement& in, account& rhs);

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

		inline const string_type&
		login() const noexcept {
			return this->_login;
		}

		inline const string_type&
		name() const noexcept {
			return this->_login;
		}

		inline const string&
		password() const noexcept {
			return this->_password;
		}

		inline const string_type&
		password_id() const noexcept {
			return this->_id;
		}

		inline unsigned int
		num_rounds() const noexcept {
			return this->_nrounds;
		}

		inline const string_type&
		password_salt() const noexcept {
			return this->_salt;
		}

		string_type
		password_prefix() const;

		static string_type
		password_prefix(
			const string_type& new_salt,
			const string_type& new_id,
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
		has_been_suspended() const noexcept {
			return this->_flags & account_flags::suspended;
		}

		inline void
		set_max_change(duration rhs) noexcept {
			this->_maxchange = rhs;
		}

		void
		clear();

		void
		set(const form_field& field, const char* value);

	private:
		void
		parse_password();

		friend struct Entity_header<account>;

	};

	std::ostream&
	operator<<(std::ostream& out, const account& rhs);

	std::istream&
	operator>>(std::istream& in, account& rhs);

	void
	operator>>(const sqlite::statement& in, account& rhs);

	template <>
	struct eq_traits<account> {

		inline static bool
		eq(const account& lhs, const account& rhs) {
			return lhs.name() == rhs.name();
		}

	};

}

namespace std {

	template<>
	struct hash<ggg::account>: public hash<string> {

		typedef size_t result_type;
		typedef ggg::account argument_type;

		inline result_type
		operator()(const argument_type& rhs) const noexcept {
			return hash<string>::operator()(rhs.login());
		}

	};

}

#endif // ACCOUNT_HH
