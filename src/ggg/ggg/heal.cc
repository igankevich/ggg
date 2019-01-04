#include "heal.hh"

#include <fcntl.h>

#include <iostream>

#include <unistdx/fs/canonical_path>
#include <unistdx/fs/file_status>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/mkdirs>
#include <unistdx/fs/path>
#include <unistdx/io/fildes>
#include <unistdx/ipc/identity>

#include <ggg/config.hh>
#include <ggg/core/acl.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>
#include <ggg/ggg/quiet_error.hh>

namespace {

	size_t num_errors = 0;

	const sys::file_mode lock_file_mode(0644);
	const sys::file_mode db_file_mode(0664);

	sys::gid_type write_gid = ggg::bad_gid;
	sys::uid_type root_uid = 0;
	sys::gid_type root_gid = 0;


	void
	change_permissions_priv(
		const char* filename,
		const sys::file_status& st,
		sys::file_mode m
	) {
		try {
			if (st.mode() != m) {
				ggg::native_message(
					std::clog,
					"Changing permissions of _ to _.",
					filename,
					m
				);
				UNISTDX_CHECK(::chmod(filename, m));
			}
		} catch (const std::exception& err) {
			++num_errors;
			ggg::native_sentence(
				std::cerr,
				"Failed to change permissions of _ to _. ",
				filename,
				m
			);
			ggg::error_message(std::cerr, err);
		}
	}

	void
	change_permissions_priv(const char* filename, sys::file_mode m) {
		sys::file_status st(filename);
		change_permissions_priv(filename, st, m);
	}

	void
	change_permissions(
		const char* filename,
		const sys::file_status& st,
		sys::file_mode m
	) {
		try {
			change_permissions_priv(filename, st, m);
		} catch (const sys::bad_call& err) {
			++num_errors;
			ggg::native_sentence(
				std::cerr,
				"Failed to change permissions of _ to _. ",
				filename,
				m
			);
			ggg::error_message(std::cerr, err);
		}
	}

	void
	change_permissions(const char* filename, sys::file_mode m) {
		try {
			change_permissions_priv(filename, m);
		} catch (const sys::bad_call& err) {
			++num_errors;
			ggg::native_sentence(
				std::cerr,
				"Failed to change permissions of _ to _. ",
				filename,
				m
			);
			ggg::error_message(std::cerr, err);
		}
	}

	void
	change_owner(
		const char* filename,
		const sys::file_status& st,
		sys::uid_type uid,
		sys::gid_type gid
	) {
		if (st.owner() != uid || st.group() != gid) {
			try {
				ggg::native_message(
					std::clog,
					"Changing owner of _ to _:_.",
					filename,
					uid,
					gid
				);
				UNISTDX_CHECK(::chown(filename, uid, gid));
			} catch (const std::exception& err) {
				++num_errors;
				ggg::native_sentence(
					std::cerr,
					"Failed to change owner of _ to _:_. ",
					filename,
					uid,
					gid
				);
				ggg::error_message(std::cerr, err);
			}
		}
	}

	void
	change_acl_priv(
		const char* filename,
		ggg::acl::access_control_list& acl,
		ggg::acl::acl_type tp = ggg::acl::default_acl
	) {
		using namespace ggg::acl;
		access_control_list old_acl(filename, tp);
		if (old_acl != acl) {
			ggg::native_message(std::clog, "Changing ACLs of _.", filename);
			acl.write(filename, tp);
		}
	}

	void
	change_acl(
		const char* filename,
		ggg::acl::access_control_list& acl,
		ggg::acl::acl_type tp
	) {
		try {
			change_acl_priv(filename, acl, tp);
		} catch (const std::exception& err) {
			++num_errors;
			ggg::native_sentence(std::cerr, "Failed to change ACLs of _. ", filename);
			ggg::error_message(std::cerr, err);
		}
	}

	void
	make_file_priv(
		const char* filename,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m,
		bool change_mode_and_owner = true
	) {
		sys::fildes tmp(
			filename,
			sys::open_flag::create | sys::open_flag::read_only,
			m
		);
		sys::file_status st(filename);
		if (!st.is_regular()) {
			ggg::native_message(std::cerr, "_ is not a regular file.", filename);
		} else if (change_mode_and_owner) {
			change_owner(filename, st, uid, gid);
			change_permissions(filename, st, m);
		}
	}

	void
	make_file(
		const char* filename,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m,
		bool change_mode_and_owner = true
	) {
		try {
			make_file_priv(filename, uid, gid, m, change_mode_and_owner);
		} catch (const sys::bad_call& err) {
			++num_errors;
			ggg::native_sentence(std::cerr, "Failed to create _. ", filename);
			ggg::error_message(std::cerr, err);
		}
	}

	void
	make_directory_priv(
		sys::path path,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m,
		bool b_change_mode = true,
		bool b_change_owner = true
	) {
		sys::file_status st;
		try {
			st.update(path);
		} catch (const sys::bad_call& err) {
			if (err.errc() != std::errc::no_such_file_or_directory) {
				throw;
			}
			ggg::native_message(std::clog, "Creating _.", path);
			sys::mkdirs(path);
			st.update(path);
		}
		if (!st.is_directory()) {
			ggg::native_message(std::cerr, "_ is not a directory.", path);
			return;
		}
		if (b_change_owner) {
			change_owner(path, st, uid, gid);
		}
		if (b_change_mode) {
			change_permissions(path, st, m);
		}
	}

	void
	make_directory(
		sys::path path,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m,
		bool b_change_mode,
		bool b_change_owner
	) {
		try {
			make_directory_priv(path, uid, gid, m, b_change_mode, b_change_owner);
		} catch (const sys::bad_call& err) {
			++num_errors;
			ggg::native_sentence(std::cerr, "Failed to create _. ", path);
			ggg::error_message(std::cerr, err);
		}
	}

	void
	make_directory(
		sys::path path,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m
	) {
		make_directory(path, uid, gid, m, true, true);
	}

	void
	heal_forms(ggg::Database& db) {
		std::vector<sys::gid_type> form_entities;
		sys::idirtree tree(sys::path(GGG_ROOT, "reg"));
		std::for_each(
			sys::idirtree_iterator<sys::directory_entry>(tree),
			sys::idirtree_iterator<sys::directory_entry>(),
			[&db,&tree,&form_entities] (const sys::directory_entry& entry) {
			    sys::path f(tree.current_dir(), entry.name());
			    sys::file_status st(f);
			    if (st.type() == sys::file_type::regular) {
			        auto rstr = db.find_user(entry.name());
					ggg::user_iterator first(rstr), last;
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
						form_entities.push_back(gid);
					}
					change_owner(f, st, uid, gid);
			        change_permissions(f, 0600);
				} else if (st.type() == sys::file_type::directory) {
					change_owner(f, st, root_uid, root_gid);
			        change_permissions(f, 0755);
				}
			}
		);
		{
			using namespace ggg::acl;
			access_control_list acl(lock_file_mode);
			for (sys::gid_type gid : form_entities) {
				acl.add_group(gid, permission_type::read_write);
			}
			if (write_gid != ggg::bad_gid) {
				acl.add_group(write_gid, permission_type::read_write);
			}
			acl.add_mask();
			change_acl(GGG_LOCK_FILE, acl, access_acl);
		}
		{
			using namespace ggg::acl;
			access_control_list acl(db_file_mode);
			for (sys::gid_type gid : form_entities) {
				acl.add_group(gid, permission_type::read_write);
			}
			if (write_gid != ggg::bad_gid) {
				acl.add_group(write_gid, permission_type::read_write);
			}
			acl.add_mask();
			change_acl(GGG_DATABASE_PATH, acl, access_acl);
		}
	}

}

void
ggg::Heal
::execute() {
	init_locale();
	#if !defined(NDEBUG)
	if (std::getenv("GGG_UNPRIVILEDGED")) {
		root_uid = sys::this_process::user();
		root_gid = sys::this_process::group();
	}
	#endif
	make_directory(sys::path(GGG_ROOT), root_uid, root_gid, 0755);
	make_directory(sys::path(GGG_ROOT, "reg"), root_uid, root_gid, 0755);
	change_owner(GGG_ROOT, sys::file_status(GGG_ROOT), root_uid, root_gid);
	change_permissions(GGG_ROOT, 0755);
	make_file(GGG_LOCK_FILE, root_uid, root_gid, lock_file_mode, false);
	ggg::Database db;
	try {
		db.open(GGG_DATABASE_PATH, false);
	} catch (const std::exception& err) {
		++num_errors;
		native_sentence(std::cerr, "Entities are broken.");
		error_message(std::cerr, err);
	}
	write_gid = db.find_id(GGG_WRITE_GROUP);
	heal_forms(db);
	if (num_errors > 0) {
		throw quiet_error();
	}
}
