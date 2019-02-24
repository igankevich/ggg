#include <chrono>
#include <ostream>
#include <locale>
#include <ctime>
#include <iomanip>
#include <locale>
#include <ratio>
#include <sstream>
#include <istream>

#include <ggg/bits/read_field.hh>
#include <ggg/core/account.hh>
#include <ggg/core/days.hh>

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

template <>
char
ggg::Entity_header<ggg::account>::delimiter() {
	return ':';
}

template <>
void
ggg::Entity_header<ggg::account>::write_header(
	std::ostream& out,
	width_container width,
	char delim
) {
	if (!width) {
		out << "LOGIN:unused:MINCHANGE:MAXCHANGE:WARNCHANGE:MAXINACTIVE:EXPIRE\n";
		return;
	}
	const char d[4] = {' ', delim, ' ', 0};
	out << std::left << std::setw(width[0]) << "LOGIN" << d
		<< std::left << std::setw(width[3]) << "MINCHANGE" << d
		<< std::left << std::setw(width[4]) << "MAXCHANGE" << d
		<< std::left << std::setw(width[5]) << "WARNCHANGE" << d
		<< std::left << std::setw(width[6]) << "MAXINACTIVE" << d
		<< std::left << std::setw(width[7]) << "EXPIRE" << '\n';
}

template <>
void
ggg::Entity_header<ggg::account>::write_body(
	std::ostream& out,
	const account& ent,
	width_container width,
	entity_format fmt,
	char delim
) {
	const char d[4] = {' ', delim, ' ', 0};
	out << std::left << std::setw(width[0]) << ent._login << d;
	if (fmt == entity_format::batch) {
		out << std::left << std::setw(width[3]) << ent._minchange << d
			<< std::left << std::setw(width[4]) << ent._maxchange << d
			<< std::left << std::setw(width[5]) << ent._warnchange << d
			<< std::left << std::setw(width[6]) << ent._maxinactive << d;
	}
	out << std::left << std::setw(width[7]) << make_formatted(&ent._expire) << '\n';
}

template <>
std::istream&
ggg::Entity_header<ggg::account>::read_body(
	std::istream& in,
	account& ent,
	entity_format fmt
) {
	std::istream::sentry s(in);
	if (!s) {
		return in;
	}
	ent.clear();
	auto fmt_expire = make_formatted(&ent._expire);
	if (fmt == entity_format::batch) {
		bits::read_all_fields(
			in, account::delimiter,
			ent._login,
			ent._minchange,
			ent._maxchange,
			ent._warnchange,
			ent._maxinactive,
			fmt_expire
		);
	} else {
		bits::read_all_fields(
			in, account::delimiter,
			ent._login,
			fmt_expire
		);
	}
	if (in.eof()) {
		in.clear();
	}
	return in;
}

