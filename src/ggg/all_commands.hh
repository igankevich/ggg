#ifndef GGG_ALL_COMMANDS_HH
#define GGG_ALL_COMMANDS_HH

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>

#include "command.hh"

namespace ggg {

	typedef std::unique_ptr<Command> command_ptr;
	typedef std::function<command_ptr()> command_ctr;

	extern std::unordered_map<std::string,command_ctr> all_commands;

	command_ptr command_from_string(const std::string& cmd);

}

#endif // GGG_ALL_COMMANDS_HH
