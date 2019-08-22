#include <iostream>

#include <unistdx/fs/file_status>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/mkdirs>
#include <unistdx/fs/path>
#include <unistdx/ipc/identity>

#include <ggg/cli/file.hh>
#include <ggg/cli/heal.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

namespace {

	size_t num_errors = 0;

	sys::file_mode root_directory_mode(0775);
	sys::file_mode entities_file_mode(0664);
	sys::file_mode accounts_file_mode(0660);

	sys::gid_type read_gid = ggg::bad_gid;
	sys::gid_type write_gid = ggg::bad_gid;
	sys::uid_type root_uid = 0;
	sys::gid_type root_gid = 0;

	sys::gid_type
	group_id(ggg::Database& db, const char* name) {
		sys::gid_type gid = ggg::bad_gid;
		try {
			gid = db.find_id(name);
		} catch (const std::exception& err) {
			gid = ggg::bad_gid;
		}
		return gid;
	}

	void
	heal_owner_permissions() {
		using namespace ggg;
        auto gid = (write_gid != bad_gid) ? write_gid : root_gid;
		File entities{GGG_ENTITIES_PATH};
        entities.owner(root_uid, gid);
        entities.mode(entities_file_mode);
		File accounts{GGG_ACCOUNTS_PATH};
        accounts.owner(root_uid, gid);
        accounts.mode(accounts_file_mode);
		Directory root{GGG_ROOT};
        root.owner(root_uid, gid);
        root.mode(root_directory_mode);
	}

	void
	determine_root_user() {
		root_uid = sys::this_process::user();
		root_gid = sys::this_process::group();
		if (root_uid != 0 && root_gid != 0) {
			accounts_file_mode = sys::file_mode(0660);
		}
	}

	void
	heal_root_directory() {
		using namespace ggg;
		Directory root{GGG_ROOT};
		root.make(root_directory_mode);
		root.owner(root_uid, root_gid);
	}

}

void
ggg::Heal
::execute() {
	init_locale();
	determine_root_user();
	heal_root_directory();
	Database db;
	try {
		{ Database(Database::File::Entities, Database::Flag::Read_write); }
		{ Database(Database::File::Accounts, Database::Flag::Read_write); }
		db.open(Database::File::All, Database::Flag::Read_write);
	} catch (const std::exception& err) {
		++num_errors;
		native_sentence(std::cerr, "Entities are broken. ");
		error_message(std::cerr, err);
	}
	read_gid = group_id(db, GGG_READ_GROUP);
	write_gid = group_id(db, GGG_WRITE_GROUP);
	heal_owner_permissions();
	if (num_errors > 0) {
		throw quiet_error();
	}
}
