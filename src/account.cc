#include "account.hh"
#include "bufcopy.hh"

#include <ratio>

namespace {

	typedef std::chrono::duration<long,std::ratio<60*60*24,1>> days;

	long
	to_days(legion::account::time_point tp) {
		using namespace std::chrono;
		return time_point_cast<days>(tp).time_since_epoch().count();
	}

	long
	to_days(legion::account::duration d) {
		using namespace std::chrono;
		return duration_cast<days>(d).count();
	}

	class if_positive {
		long _value;
	public:
		explicit if_positive(long v): _value(v) {}
		friend std::ostream&
		operator<<(std::ostream& out, const if_positive& rhs) {
			if (rhs._value > 0) {
				out << rhs._value;
			}
			return out;
		}
	};


}

size_t
legion::account::buffer_size() const noexcept {
	return this->_login.size() + 1
		+ this->_password.size() + 1;
}

void
legion::account::copy_to(struct ::spwd* lhs, char* buffer) const {
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
legion::operator<<(std::ostream& out, const account& rhs) {
	return out
		<< rhs._login << ':'
		<< rhs._password << ':'
		<< if_positive(to_days(rhs._lastchange)) << ':'
		<< if_positive(to_days(rhs._minchange)) << ':'
		<< if_positive(to_days(rhs._maxchange)) << ':'
		<< if_positive(to_days(rhs._warnchange)) << ':'
		<< if_positive(to_days(rhs._maxinactive)) << ':'
		<< if_positive(to_days(rhs._expire));
}
