#ifndef CORE_FORM_TYPE_HH
#define CORE_FORM_TYPE_HH

#include <iosfwd>
#include <string>

namespace ggg {

	enum struct form_type {
		console,
		graphical
	};

	std::ostream&
	operator<<(std::ostream& out, form_type rhs);

	std::istream&
	operator>>(std::istream& in, form_type& rhs);

	form_type
	from_string(const std::string& rhs);

}

#endif // vim:filetype=cpp
