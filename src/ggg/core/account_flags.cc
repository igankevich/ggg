#include "account_flags.hh"
#include <bitset>
#include <limits>

std::ostream&
ggg::operator<<(std::ostream& out, const account_flags& rhs) {
	typedef std::underlying_type<account_flags>::type tp;
	static_assert(!std::numeric_limits<tp>::is_signed, "bad type");
	std::bitset<std::numeric_limits<tp>::digits> bits{tp(rhs)};
	if (bits.any()) {
		out << bits;
	}
	return out;
}

std::istream&
ggg::operator>>(std::istream& in, account_flags& rhs) {
	typedef std::underlying_type<account_flags>::type tp;
	static_assert(!std::numeric_limits<tp>::is_signed, "bad type");
	std::bitset<std::numeric_limits<tp>::digits> bits;
	if (in >> bits) {
		rhs = account_flags(static_cast<tp>(bits.to_ulong()));
	}
	return in;
}
