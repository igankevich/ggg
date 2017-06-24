#include "account.hh"
#include "bufcopy.hh"
#include "read_field.hh"

#include <ratio>

namespace {

	typedef std::chrono::duration<long,std::ratio<60*60*24,1>> days;
	typedef std::chrono::time_point<ggg::account::clock_type,days>
		time_point_in_days;

	long
	to_days(ggg::account::time_point tp) {
		using namespace std::chrono;
		return time_point_cast<days>(tp).time_since_epoch().count();
	}

	long
	to_days(ggg::account::duration d) {
		using namespace std::chrono;
		return duration_cast<days>(d).count();
	}

	ggg::account::time_point
	time_point_from_days(long d) {
		using namespace std::chrono;
		return time_point_cast<ggg::account::duration>(
			time_point_in_days(days(d))
		);
	}

	ggg::account::duration
	duration_from_days(long d) {
		using namespace std::chrono;
		return duration_cast<ggg::account::duration>(days(d));
	}

}

namespace ggg {

	std::ostream&
	operator<<(std::ostream& out, const ggg::account::time_point& rhs) {
		long d = to_days(rhs);
		if (d > 0) {
			out << d;
		}
		return out;
	}

	std::ostream&
	operator<<(std::ostream& out, const ggg::account::duration& rhs) {
		long d = to_days(rhs);
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
		rhs = time_point_from_days(val);
		return in;
	}

	std::istream&
	operator>>(std::istream& in, ggg::account::duration& rhs) {
		long val;
		if (!(in >> val)) {
			val = 0L;
			in.clear();
		}
		rhs = duration_from_days(val);
		return in;
	}

}

size_t
ggg::account::buffer_size() const noexcept {
	return this->_login.size() + 1
		+ this->_password.size() + 1;
}

void
ggg::account::copy_to(struct ::spwd* lhs, char* buffer) const {
	buffer = bits::bufcopy(&lhs->sp_namp, buffer, this->_login.data());
	buffer = bits::bufcopy(&lhs->sp_pwdp, buffer, this->_password.data());
	lhs->sp_lstchg = to_days(this->_lastchange);
	lhs->sp_min = to_days(this->_minchange);
	lhs->sp_max = to_days(this->_maxchange);
	lhs->sp_warn = to_days(this->_warnchange);
	lhs->sp_inact = to_days(this->_maxinactive);
	lhs->sp_expire = to_days(this->_expire);
}

std::ostream&
ggg::operator<<(std::ostream& out, const account& rhs) {
	return out
		<< rhs._login << ':'
		<< rhs._password << ':'
		<< rhs._lastchange << ':'
		<< rhs._minchange << ':'
		<< rhs._maxchange << ':'
		<< rhs._warnchange << ':'
		<< rhs._maxinactive << ':'
		<< rhs._expire << ':' /* ignore sp_flag */;
}

std::istream&
ggg::operator>>(std::istream& in, account& rhs) {
	std::istream::sentry s(in);
	if (s) {
		bits::read_all_fields(
			in, ':',
			rhs._login,
			rhs._password,
			rhs._lastchange,
			rhs._minchange,
			rhs._maxchange,
			rhs._warnchange,
			rhs._maxinactive,
			rhs._expire
		);
		if (in.eof()) {
			in.clear();
		}
	}
	return in;
}
