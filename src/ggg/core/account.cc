#include <ctime>
#include <iomanip>
#include <locale>
#include <ratio>

#include <ggg/bits/read_field.hh>
#include <ggg/core/account.hh>
#include <ggg/core/days.hh>
#include <ggg/sec/secure_sstream.hh>

namespace {

	template <class T>
	class formatted_date {

	private:
		T _date = nullptr;

	public:
		formatted_date() = default;

		explicit
		formatted_date(T date):
		_date(date)
		{}

		friend std::ostream&
		operator<<(std::ostream& out, const formatted_date& rhs) {
			if (!rhs._date) {
				return out;
			}
			typedef std::time_put<char> tput_type;
			typedef ggg::account::time_point tp;
			typedef ggg::account::duration dur;
			if (*rhs._date > tp(dur::zero())) {
				std::time_t t = ggg::account::clock_type::to_time_t(*rhs._date);
				std::tm* tm = std::gmtime(&t);
				const tput_type& tput = std::use_facet<tput_type>(out.getloc());
				tput.put(out, out, ' ', tm, 'F');
			}
			return out;
		}

		friend std::istream&
		operator>>(std::istream& in, formatted_date& rhs) {
			using ggg::days;
			if (!rhs._date) {
				return in;
			}
			typedef std::time_get<char> tget_type;
			const tget_type& tget = std::use_facet<tget_type>(in.getloc());
			std::ios::iostate state = std::ios::goodbit;
			std::tm tm{};
			std::string tmp;
			in >> tmp;
			std::istringstream iss(tmp);
			std::string fmt("%Y-%m-%d");
			tget.get(iss, tget_type::iter_type(), iss, state, &tm, fmt.data(), fmt.data() + fmt.length());
			std::time_t t = std::mktime(&tm);
			*rhs._date = ggg::account::clock_type::from_time_t(t) + days(1);
			return in;
		}
	};

	template <class T>
	formatted_date<T*>
	make_formatted(T* rhs) {
		return formatted_date<T*>(rhs);
	}

	template <class T>
	formatted_date<const T*>
	make_formatted(const T* rhs) {
		return formatted_date<const T*>(rhs);
	}

	template <class T>
	void
	read_field(T& field, const char* value, const char* err) {
		std::stringstream str(value);
		str >> field;
		if (str.fail()) {
			throw std::invalid_argument(err);
		}
	}

}

namespace std {
namespace chrono {

	std::ostream&
	operator<<(std::ostream& out, const ggg::account::time_point& rhs) {
		long d = ggg::to_days(rhs);
		if (d > 0) {
			out << d;
		}
		return out;
	}

	std::ostream&
	operator<<(std::ostream& out, const ggg::account::duration& rhs) {
		long d = ggg::to_days(rhs);
		if (d > 0) {
			out << d;
		}
		return out;
	}

	std::istream&
	operator>>(std::istream& in, ggg::account::time_point& rhs) {
		long val;
		if (!(in >> val)) {
			val = 0L;
			in.clear();
		}
		rhs = ggg::time_point_from_days(val);
		return in;
	}

	std::istream&
	operator>>(std::istream& in, ggg::account::duration& rhs) {
		long val;
		if (!(in >> val)) {
			val = 0L;
			in.clear();
		}
		rhs = ggg::duration_from_days(val);
		return in;
	}

}
}

std::ostream&
ggg::operator<<(std::ostream& out, const account& rhs) {
	return out
		<< rhs._login << account::delimiter
		<< rhs._password << account::delimiter
		<< rhs._lastchange << account::delimiter
		<< rhs._minchange << account::delimiter
		<< rhs._maxchange << account::delimiter
		<< rhs._warnchange << account::delimiter
		<< rhs._maxinactive << account::delimiter
		<< rhs._expire << account::delimiter
		<< rhs._flags;
}

std::istream&
ggg::operator>>(std::istream& in, account& rhs) {
	std::istream::sentry s(in);
	if (s) {
		rhs.clear();
		bits::read_all_fields(
			in, account::delimiter,
			rhs._login,
			rhs._password,
			rhs._lastchange,
			rhs._minchange,
			rhs._maxchange,
			rhs._warnchange,
			rhs._maxinactive,
			rhs._expire,
			rhs._flags
		);
		if (in.eof()) {
			in.clear();
		}
	}
	return in;
}

void
ggg::account::copy_from(const account& rhs) {
	this->_login = rhs._login;
	this->_minchange = rhs._minchange;
	this->_maxchange = rhs._maxchange;
	this->_warnchange = rhs._warnchange;
	this->_maxinactive = rhs._maxinactive;
	this->_expire = rhs._expire;
}

void
ggg::account::set_password(string_type&& rhs) {
	this->_password = std::move(rhs);
	this->_lastchange = clock_type::now();
	this->unsetf(account_flags::password_has_expired);
}

void
ggg::account::clear() {
	this->_login.clear();
	this->_password.clear();
	this->_lastchange = time_point(duration::zero());
	this->_minchange = duration::zero();
	this->_maxchange = duration::zero();
	this->_warnchange = duration::zero();
	this->_maxinactive = duration::zero();
	this->_expire = time_point(duration::zero());
	this->_flags = account_flags(0);
}

void
ggg::operator>>(const sqlite::statement& in, account& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	uint64_t flags = 0;
	cstr >> rhs._login >> rhs._password >> rhs._expire >> flags;
	rhs._flags = static_cast<account_flags>(flags);
}

