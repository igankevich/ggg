#ifndef GGG_COMMAND_HH
#define GGG_COMMAND_HH

#include <istream>
#include <ostream>

namespace ggg {

	enum struct Command: unsigned int {
		Add = 0,
		Delete = 1,
		Help = 2,
		Version = 3
	};

	Command
	command_from_string(const std::string& rhs);

	std::ostream&
	operator<<(std::ostream& out, Command rhs);

	std::istream&
	operator>>(std::istream& in, Command& rhs);

}

#endif // GGG_COMMAND_HH
