#ifndef GGG_ALL_COMMANDS_HH
#define GGG_ALL_COMMANDS_HH

#include <string>
#include <memory>

#include "command.hh"

namespace ggg {

	typedef std::unique_ptr<Command> command_ptr;

	command_ptr command_from_string(const std::string& cmd);

}

#endif // GGG_ALL_COMMANDS_HH
