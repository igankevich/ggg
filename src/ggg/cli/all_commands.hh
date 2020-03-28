#ifndef GGG_CLI_ALL_COMMANDS_HH
#define GGG_CLI_ALL_COMMANDS_HH

#include <string>
#include <memory>

#include "command.hh"

namespace ggg {

    typedef std::unique_ptr<Command> command_ptr;

    command_ptr command_from_string(const std::string& cmd);

}

#endif // vim:filetype=cpp
