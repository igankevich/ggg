#ifndef GGG_CORE_DAYS_HH
#define GGG_CORE_DAYS_HH

#include <chrono>

namespace ggg {

	typedef std::chrono::duration<long,std::ratio<60*60*24,1>> days;
	typedef std::chrono::system_clock clock_type;
	typedef std::chrono::time_point<clock_type,days> time_point_in_days;

	inline long
	to_days(clock_type::time_point tp) {
		using namespace std::chrono;
		return time_point_cast<days>(tp).time_since_epoch().count();
	}

	inline long
	to_days(clock_type::duration d) {
		using namespace std::chrono;
		return duration_cast<days>(d).count();
	}

	inline clock_type::time_point
	time_point_from_days(long d) {
		using namespace std::chrono;
		typedef clock_type::duration duration;
		return time_point_cast<duration>(time_point_in_days(days(d)));
	}

	inline void
	set_days(long& lhs, clock_type::time_point rhs) {
		typedef clock_type::duration dur;
		typedef clock_type::time_point tp;
		if (rhs > tp(dur::zero())) {
			lhs = to_days(rhs);
		}
	}

	inline void
	set_days(long& lhs, clock_type::duration rhs) {
		typedef clock_type::duration dur;
		if (rhs > dur::zero()) {
			lhs = to_days(rhs);
		}
	}

	inline clock_type::duration
	duration_from_days(long d) {
		using namespace std::chrono;
		return duration_cast<clock_type::duration>(days(d));
	}

}

#endif // vim:filetype=cpp
