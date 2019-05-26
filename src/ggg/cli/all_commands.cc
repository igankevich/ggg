#include "all_commands.hh"

#include <stdexcept>
#include <functional>
#include <unordered_map>

#include <ggg/cli/add_entity.hh>
#include <ggg/cli/attach.hh>
#include <ggg/cli/backup.hh>
#include <ggg/cli/dot.hh>
#include <ggg/cli/edit_entity.hh>
#include <ggg/cli/expire_entity.hh>
#include <ggg/cli/expunge.hh>
#include <ggg/cli/find_entities.hh>
#include <ggg/cli/find_machines.hh>
#include <ggg/cli/guile.hh>
#include <ggg/cli/heal.hh>
#include <ggg/cli/lock_entity.hh>
#include <ggg/cli/remove_entity.hh>
#include <ggg/cli/reset_password.hh>
#include <ggg/cli/sanitise.hh>
#include <ggg/cli/show_duplicates.hh>
#include <ggg/cli/show_entity.hh>
#include <ggg/cli/show_expired.hh>
#include <ggg/cli/show_groups.hh>
#include <ggg/cli/show_help.hh>
#include <ggg/cli/show_locked.hh>
#include <ggg/cli/show_members.hh>
#include <ggg/cli/show_version.hh>
#include <ggg/cli/unlock_entity.hh>

#define MAKE_COMMAND(name, type) \
	{ \
		name, \
		[](){ \
			using namespace ::ggg; \
			return command_ptr(new type); \
		} \
	}

namespace {

	typedef std::function<ggg::command_ptr()> command_ctr;

	std::unordered_map<std::string,command_ctr> all_commands{
		MAKE_COMMAND("init", Heal),
		MAKE_COMMAND("heal", Heal),
		MAKE_COMMAND("backup", Backup),
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
		MAKE_COMMAND("show", Show_entity),
		MAKE_COMMAND("info", Show_entity),
		MAKE_COMMAND("members", Show_members),
		MAKE_COMMAND("children", Show_members),
		MAKE_COMMAND("groups", Show_groups),
		MAKE_COMMAND("parents", Show_groups),
		MAKE_COMMAND("find", Find_entities),
		MAKE_COMMAND("search", Find_entities),
		MAKE_COMMAND("expired", Show_expired),
		MAKE_COMMAND("locked", Show_locked),
		MAKE_COMMAND("inactive", Show_locked),
		MAKE_COMMAND("sanitise", Sanitise),
		MAKE_COMMAND("sanitize", Sanitise),
		MAKE_COMMAND("clean", Sanitise),
		MAKE_COMMAND("expunge", Expunge),
		MAKE_COMMAND("purge", Expunge),
		MAKE_COMMAND("attach", Attach),
		MAKE_COMMAND("detach", Detach),
		MAKE_COMMAND("tie", Tie),
		MAKE_COMMAND("untie", Untie),
		MAKE_COMMAND("dot", Dot),
		MAKE_COMMAND("graphviz", Dot),
		MAKE_COMMAND("machines", Find_machines),
		MAKE_COMMAND("hosts", Find_machines),
		MAKE_COMMAND("guile", Guile),
		MAKE_COMMAND("script", Guile),
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

