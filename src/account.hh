#ifndef ACCOUNT_HH
#define ACCOUNT_HH

#include <shadow.h>
#include <string>
#include <chrono>
#include <ostream>

namespace legion {

	class account {

	public:
		typedef std::chrono::system_clock clock_type;
		typedef clock_type::time_point time_point;
		typedef clock_type::duration duration;

	private:
		std::string _login;
		std::string _password;
		time_point _lastchange;
		duration _minchange;
		duration _maxchange;
		duration _warnchange;
		duration _maxinactive;
		time_point _expire;

	public:
		account() = default;
		account(const account&) = default;
		account(account&&) = default;
		account& operator=(const account&) = default;

		size_t
		buffer_size() const noexcept;

		void
		copy_to(struct ::spwd* lhs, char* buffer) const;

		friend std::ostream&
		operator<<(std::ostream& out, const account& rhs);
	};

	std::ostream&
	operator<<(std::ostream& out, const account& rhs);

}

#endif // ACCOUNT_HH
