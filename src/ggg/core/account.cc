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
		rhs.parse_password();
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
ggg::account::parse_password() {
	if (this->_password.empty() || this->_password.front() != separator) {
		return;
	}
	typedef string::traits_type traits_type;
	const char rounds[] = "rounds=";
	const ptrdiff_t rounds_size = sizeof(rounds) - 1;
	const char* first = this->_password.data() + 1;
	const char* last = this->_password.data() + this->_password.size();
	const char* prev = first;
	size_t field_no = 0;
	while (first != last) {
		if (*first == separator) {
			if (field_no == 0) {
				this->_id.assign(prev, first);
			} else if (0 == traits_type::compare(
				rounds,
				prev,
				std::min(rounds_size, first-prev)
			)) {
				secure_stringstream str(string(prev+rounds_size, first));
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

ggg::account::string_type
ggg::account::password_prefix() const {
	return password_prefix(this->_salt, this->_id, this->_nrounds);
}

ggg::account::string_type
ggg::account::password_prefix(
	const string_type& new_salt,
	const string_type& new_id,
	unsigned int nrounds
) {
	std::stringstream s;
	s.put(separator);
	s << new_id;
	s.put(separator);
	if (nrounds > 0) {
		s << "rounds=" << nrounds;
		s.put(separator);
	}
	s << new_salt;
	s.put(separator);
	return s.str();
}

void
ggg::account::set_password(const string& rhs) {
	this->_password = rhs;
	this->_lastchange = clock_type::now();
	this->unsetf(account_flags::password_has_expired);
}

void
ggg::account::clear() {
	this->_login.clear();
	this->_id.clear();
	this->_salt.clear();
	this->_nrounds = 0;
	this->_password.clear();
	this->_lastchange = time_point(duration::zero());
	this->_minchange = duration::zero();
	this->_maxchange = duration::zero();
	this->_warnchange = duration::zero();
	this->_maxinactive = duration::zero();
	this->_expire = time_point(duration::zero());
	this->_flags = account_flags(0);
	this->_origin.clear();
}

void
ggg::account::set(const form_field& field, const char* value) {
	const std::string& t = field.target();
	if (t == "account.login") {
		this->_login = value;
	} else if (t == "account.lastchange") {
		read_field(this->_lastchange, value, "bad lastchange");
	} else if (t == "account.minchange") {
		read_field(this->_minchange, value, "bad minchange");
	} else if (t == "account.maxchange") {
		read_field(this->_maxchange, value, "bad maxchange");
	} else if (t == "account.warnchange") {
		read_field(this->_warnchange, value, "bad warnchange");
	} else if (t == "account.maxinactive") {
		read_field(this->_maxinactive, value, "bad maxinactive");
	} else if (t == "account.expire") {
		auto fmt_expire = make_formatted(&this->_expire);
		read_field(fmt_expire, value, "bad expire");
	} else if (t == "account.flags") {
		read_field(this->_flags, value, "bad flags");
	} else if (t == "account.origin") {
		this->_origin = value;
	} else {
		std::stringstream msg;
		msg << "bad field target: \"" << t << "\"";
		throw std::invalid_argument(msg.str());
	}
}

sqlite::rstream&
ggg::operator>>(sqlite::rstream& in, account& rhs) {
	rhs.clear();
	sqlite::cstream cstr(in);
	if (in >> cstr) {
		uint64_t flags = 0;
		cstr >> rhs._login >> rhs._password >> rhs._expire >> flags;
		rhs._flags = static_cast<account_flags>(flags);
		rhs.parse_password();
	}
	return in;
}

