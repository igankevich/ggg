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

	sys::file_mode root_directory_mode(0755);
	sys::file_mode entities_file_mode(0644);
	sys::file_mode accounts_file_mode(0000);
	sys::file_mode form_file_mode(0600);

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
	heal_forms(ggg::Database& db) {
		using namespace ggg;
		sys::idirtree tree(sys::path(GGG_ROOT, "reg"));
		std::for_each(
			sys::idirtree_iterator<sys::directory_entry>(tree),
			sys::idirtree_iterator<sys::directory_entry>(),
			[&db,&tree] (const sys::directory_entry& entry) {
			    sys::path f(tree.current_dir(), entry.name());
			    sys::file_status st(f);
			    if (st.type() == sys::file_type::regular) {
			        auto rstr = db.find_user(entry.name());
					user_iterator first(rstr), last;
			        sys::uid_type uid = root_uid;
			        sys::gid_type gid = root_gid;
			        if (first == last) {
						ggg::native_message(
							std::cerr,
							"Unable to find form entity _.",
							entry.name()
						);
					} else {
			            uid = first->id();
			            gid = first->gid();
					}
					File file{f};
					file.owner(uid, gid);
					file.mode(form_file_mode);
				} else if (st.type() == sys::file_type::directory) {
					Directory dir{f};
					dir.owner(root_uid, root_gid);
					dir.mode(0755);
				}
			}
		);
	}

	void
	heal_access_control_lists() {
		using namespace ggg::acl;
		using namespace ggg;
		access_control_list root(root_directory_mode);
		access_control_list entities(entities_file_mode);
		access_control_list accounts(accounts_file_mode);
		if (write_gid != ggg::bad_gid) {
			entities.add_group(write_gid, permission_type::read_write);
			accounts.add_group(write_gid, permission_type::read_write);
			root.add_group(write_gid, permission_type::read_write_execute);
		}
		if (read_gid != ggg::bad_gid) {
			accounts.add_group(read_gid, permission_type::read);
		}
		File{GGG_ENTITIES_PATH}.mode(entities);
		File{GGG_ACCOUNTS_PATH}.mode(accounts);
		Directory{GGG_ROOT}.mode(root);
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
		Directory reg{sys::path(GGG_ROOT, "reg")};
		reg.make(0755);
		reg.owner(root_uid, root_gid);
	}

	void
	heal_database() {
		using namespace ggg;
		File{GGG_ENTITIES_PATH}.owner(root_uid, root_gid);
		File{GGG_ACCOUNTS_PATH}.owner(root_uid, root_gid);
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
	heal_database();
	read_gid = group_id(db, GGG_READ_GROUP);
	write_gid = group_id(db, GGG_WRITE_GROUP);
	heal_forms(db);
	heal_access_control_lists();
	if (num_errors > 0) {
		throw quiet_error();
	}
}
