#include "all_commands.hh"

#include <stdexcept>
#include <functional>
#include <unordered_map>

#include "add_entity.hh"
#include "edit_entity.hh"
#include "expire_entity.hh"
#include "heal.hh"
#include "remove_entity.hh"
#include "reset_password.hh"
#include "restore_entity.hh"
#include "show_duplicates.hh"
#include "show_help.hh"
#include "show_version.hh"

#define MAKE_COMMAND(name, type) \
	{name, [](){ return ::ggg::command_ptr(new ::ggg::type); }}

namespace {

	typedef std::function<ggg::command_ptr()> command_ctr;

	std::unordered_map<std::string,command_ctr> all_commands{
		MAKE_COMMAND("delete", Remove_entity),
		MAKE_COMMAND("remove", Remove_entity),
		MAKE_COMMAND("rm", Remove_entity),
		MAKE_COMMAND("expire", Expire_entity),
		MAKE_COMMAND("lock", Expire_entity),
		MAKE_COMMAND("deactivate", Expire_entity),
		MAKE_COMMAND("restore", Restore_entity),
		MAKE_COMMAND("activate", Restore_entity),
		MAKE_COMMAND("unlock", Restore_entity),
		MAKE_COMMAND("reset", Reset_password),
		MAKE_COMMAND("duplicates", Show_duplicates),
		MAKE_COMMAND("edit", Edit_entity),
		MAKE_COMMAND("modify", Edit_entity),
		MAKE_COMMAND("change", Edit_entity),
		MAKE_COMMAND("add", Add_entity),
		MAKE_COMMAND("insert", Add_entity),
		MAKE_COMMAND("new", Add_entity),
		MAKE_COMMAND("version", Show_version),
		MAKE_COMMAND("heal", Heal),
		MAKE_COMMAND("init", Heal),
		MAKE_COMMAND("-v", Show_version),
		MAKE_COMMAND("--version", Show_version),
		MAKE_COMMAND("help", Show_help),
		MAKE_COMMAND("-h", Show_help),
		MAKE_COMMAND("--help", Show_help),
	};

}

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

