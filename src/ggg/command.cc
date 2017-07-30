#include "command.hh"

#include <array>
#include <type_traits>
#include <string>
#include <stdexcept>

namespace {

	std::array<const char*,3> command_names{{
		"add",
		"delete",
		"help"
	}};

}


ggg::Command
ggg::command_from_string(const std::string& rhs) {
	typedef std::underlying_type<Command>::type tp;
	Command cmd;
	bool success = false;
	for (tp i=0; i<command_names.size(); ++i) {
		if (rhs == command_names[i]) {
			cmd = Command(i);
			success = true;
			break;
		}
	}
	if (!success) {
		throw std::runtime_error("bad command");
	}
	return cmd;
}

std::ostream&
ggg::operator<<(std::ostream& out, Command rhs) {
	typedef std::underlying_type<Command>::type tp;
	const tp idx = static_cast<tp>(rhs);
	const char* name;
	if (idx >= command_names.size()) {
		name = "unknown";
	} else {
		name = command_names[idx];
	}
	return out << name;
}

std::istream&
ggg::operator>>(std::istream& in, Command& rhs) {
	std::string str;
	if (in >> str) {
		rhs = command_from_string(str);
	}
	return in;
}
