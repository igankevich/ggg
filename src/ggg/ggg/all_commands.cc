#include "all_commands.hh"

#include <stdexcept>
#include <functional>
#include <unordered_map>

#include <ggg/ggg/add_entity.hh>
#include <ggg/ggg/copy.hh>
#include <ggg/ggg/edit_entity.hh>
#include <ggg/ggg/expire_entity.hh>
#include <ggg/ggg/expunge.hh>
#include <ggg/ggg/find_entities.hh>
#include <ggg/ggg/heal.hh>
#include <ggg/ggg/lock_entity.hh>
#include <ggg/ggg/remove_entity.hh>
#include <ggg/ggg/reset_password.hh>
#include <ggg/ggg/sanitise.hh>
#include <ggg/ggg/show_duplicates.hh>
#include <ggg/ggg/show_entity.hh>
#include <ggg/ggg/show_expired.hh>
#include <ggg/ggg/show_groups.hh>
#include <ggg/ggg/show_help.hh>
#include <ggg/ggg/show_locked.hh>
#include <ggg/ggg/show_members.hh>
#include <ggg/ggg/show_version.hh>
#include <ggg/ggg/test_lock.hh>
#include <ggg/ggg/unlock_entity.hh>

#define MAKE_COMMAND(name, type) \
	{name, [](){ return ::ggg::command_ptr(new ::ggg::type); }}

namespace {

	typedef std::function<ggg::command_ptr()> command_ctr;

	std::unordered_map<std::string,command_ctr> all_commands{
		MAKE_COMMAND("delete", Remove_entity),
		MAKE_COMMAND("remove", Remove_entity),
		MAKE_COMMAND("rm", Remove_entity),
		MAKE_COMMAND("expire", Expire_entity),
		MAKE_COMMAND("lock", Lock_entity),
		MAKE_COMMAND("suspend", Lock_entity),
		MAKE_COMMAND("deactivate", Lock_entity),
		MAKE_COMMAND("unlock", Unlock_entity),
		MAKE_COMMAND("resume", Unlock_entity),
		MAKE_COMMAND("activate", Unlock_entity),
		MAKE_COMMAND("reset", Reset_password),
		MAKE_COMMAND("duplicates", Show_duplicates),
		MAKE_COMMAND("edit", Edit_entity),
		MAKE_COMMAND("modify", Edit_entity),
		MAKE_COMMAND("change", Edit_entity),
		MAKE_COMMAND("add", Add_entity),
		MAKE_COMMAND("insert", Add_entity),
		MAKE_COMMAND("new", Add_entity),
		MAKE_COMMAND("version", Show_version),
		MAKE_COMMAND("copy", Copy),
		MAKE_COMMAND("heal", Heal),
		MAKE_COMMAND("init", Heal),
		MAKE_COMMAND("show", Show_entity),
		MAKE_COMMAND("info", Show_entity),
		MAKE_COMMAND("members", Show_members),
		MAKE_COMMAND("team", Show_members),
		MAKE_COMMAND("find", Find_entities),
		MAKE_COMMAND("search", Find_entities),
		MAKE_COMMAND("groups", Show_groups),
		MAKE_COMMAND("expired", Show_expired),
		MAKE_COMMAND("locked", Show_locked),
		MAKE_COMMAND("inactive", Show_locked),
		MAKE_COMMAND("sanitise", Sanitise),
		MAKE_COMMAND("sanitize", Sanitise),
		MAKE_COMMAND("clean", Sanitise),
		MAKE_COMMAND("test-lock", Test_lock),
		MAKE_COMMAND("testlock", Test_lock),
		MAKE_COMMAND("expunge", Expunge),
		MAKE_COMMAND("purge", Expunge),
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

