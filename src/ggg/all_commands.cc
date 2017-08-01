#include "all_commands.hh"

#include <stdexcept>

#include "remove_entity.hh"
#include "show_help.hh"
#include "show_version.hh"
#include "expire_entity.hh"
#include "restore_entity.hh"

#define MAKE_COMMAND(name, type) \
	{name, [] () { return ::ggg::command_ptr(new type); }}

std::unordered_map<std::string,ggg::command_ctr> ggg::all_commands{
	MAKE_COMMAND("delete", Remove_entity),
	MAKE_COMMAND("remove", Remove_entity),
	MAKE_COMMAND("rm", Remove_entity),
	MAKE_COMMAND("expire", Expire_entity),
	MAKE_COMMAND("lock", Expire_entity),
	MAKE_COMMAND("deactivate", Expire_entity),
	MAKE_COMMAND("restore", Restore_entity),
	MAKE_COMMAND("activate", Restore_entity),
	MAKE_COMMAND("unlock", Restore_entity),
	MAKE_COMMAND("version", Show_version),
	MAKE_COMMAND("-v", Show_version),
	MAKE_COMMAND("--version", Show_version),
	MAKE_COMMAND("help", Show_help),
	MAKE_COMMAND("-h", Show_help),
	MAKE_COMMAND("--help", Show_help),
};

#undef MAKE_COMMAND

ggg::command_ptr
ggg::command_from_string(const std::string& cmd) {
	auto result = all_commands.find(cmd);
	if (result == all_commands.end()) {
		throw std::invalid_argument("bad command");
	}
	command_ptr ptr = result->second();
	ptr->prefix(cmd);
	return ptr;
}

