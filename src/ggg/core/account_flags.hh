#ifndef ACCOUNT_FLAGS_HH
#define ACCOUNT_FLAGS_HH

#include <istream>
#include <ostream>

namespace ggg {

	enum account_flags: uint64_t {
		suspended = 1,
		password_has_expired = 2
	};

	#define BINARY_OP(op) \
	inline account_flags \
	operator op(account_flags lhs, account_flags rhs) noexcept { \
		typedef std::underlying_type<account_flags>::type tp; \
		return account_flags(tp(lhs) op tp(rhs)); \
	}

	#define UNARY_OP(op) \
	inline account_flags \
	operator op(account_flags rhs) noexcept { \
		typedef std::underlying_type<account_flags>::type tp; \
		return account_flags(op tp(rhs)); \
	}

	BINARY_OP(&)
	BINARY_OP(|)
	UNARY_OP(~)

	#undef BINARY_OP
	#undef UNARY_OP

	std::ostream&
	operator<<(std::ostream& out, const account_flags& rhs);

	std::istream&
	operator>>(std::istream& in, account_flags& rhs);

}

#endif // ACCOUNT_FLAGS_HH
