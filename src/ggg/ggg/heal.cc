#include "heal.hh"

#include <fcntl.h>

#include <iostream>

#include <unistdx/fs/canonical_path>
#include <unistdx/fs/file_status>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/mkdirs>
#include <unistdx/fs/path>
#include <unistdx/io/fildes>

#include <ggg/config.hh>
#include <ggg/core/acl.hh>
#include <ggg/core/native.hh>
#include <ggg/ggg/quiet_error.hh>

namespace {

	size_t num_errors = 0;

	const sys::file_mode lock_file_mode(0644);
	sys::gid_type read_gid = 0;
	sys::gid_type write_gid = 0;


	bool
	has_ggg_groups() {
		return read_gid != 0 || write_gid != 0;
	}

	void
	add_directory_acl(ggg::acl::access_control_list& acl) {
		using namespace ggg::acl;
		if (read_gid != 0) {
			acl.add_group(read_gid, permission_type::read);
		}
		if (write_gid != 0) {
			acl.add_group(write_gid, permission_type::read_write);
		}
	}

	void
	find_group(
		const ggg::Hierarchy& h,
		const char* name,
		sys::gid_type& gid
	) {
		auto result = h.find_by_name(name);
		if (result != h.end()) {
		    gid = result->gid();
		}
	}

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
		const char* root,
		std::string name,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m,
		bool b_change_mode = true,
		bool b_change_owner = true
	) {
		sys::path p(root, name);
		sys::file_status st;
		try {
			st.update(p);
		} catch (const sys::bad_call& err) {
			if (err.errc() != std::errc::no_such_file_or_directory) {
				throw;
			}
			name += '/';
			ggg::native_message(std::clog, "Creating _.", p);
			sys::mkdirs(p);
			st.update(p);
		}
		if (!st.is_directory()) {
			ggg::native_message(std::cerr, "_ is not a directory.", p);
			return;
		}
		if (b_change_owner) {
			change_owner(p, st, uid, gid);
		}
		if (b_change_mode) {
			change_permissions(p, st, m);
		}
	}

	void
	make_directory(
		const char* root,
		std::string name,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m,
		bool b_change_mode,
		bool b_change_owner
	) {
		try {
			make_directory_priv(root, name, uid, gid, m, b_change_mode, b_change_owner);
		} catch (const sys::bad_call& err) {
			sys::path p(root, name);
			++num_errors;
			ggg::native_sentence(std::cerr, "Failed to create _. ", p);
			ggg::error_message(std::cerr, err);
		}
	}

	void
	make_directory(
		const char* root,
		std::string name,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m
	) {
		make_directory(root, name, uid, gid, m, true, true);
	}

	void
	make_directory(
		const char* root,
		std::string name,
		sys::uid_type uid,
		sys::gid_type gid
	) {
		make_directory(root, name, uid, gid, sys::file_mode{0755}, false, true);
	}

	void
	make_directory(const char* root, std::string name) {
		make_directory(root, name, 0, 0, sys::file_mode{0755}, false, false);
	}

	void
	heal_entities_subdir(sys::path dir, sys::uid_type uid, sys::gid_type gid) {
		sys::idirtree tree(dir);
		std::for_each(
			sys::idirtree_iterator<sys::directory_entry>(tree),
			sys::idirtree_iterator<sys::directory_entry>(),
			[uid,gid,&tree] (const sys::directory_entry& entry) {
			    sys::path f(tree.current_dir(), entry.name());
			    sys::file_status st(f);
				change_owner(f, st, uid, gid);
			    if (st.type() == sys::file_type::regular) {
			        change_permissions(f, 0644);
				} else if (st.type() == sys::file_type::directory) {
			        change_permissions(f, 0755);
				}
			}
		);
	}

	void
	heal_entities(const ggg::Hierarchy& h) {
		sys::idirectory dir(sys::path(GGG_ROOT, "ent"));
		std::for_each(
			sys::idirectory_iterator<sys::directory_entry>(dir),
			sys::idirectory_iterator<sys::directory_entry>(),
			[&h,&dir] (const sys::directory_entry& entry) {
			    sys::path f(dir.getpath(), entry.name());
			    sys::file_status st(f);
			    if (st.type() == sys::file_type::regular) {
					change_owner(f, st, 0, 0);
			        change_permissions(f, 0644);
				} else if (st.type() == sys::file_type::directory) {
			        auto result = h.find_by_name(entry.name());
			        sys::uid_type uid = 0;
			        sys::gid_type gid = 0;
			        if (result != h.end()) {
			            uid = result->id();
			            gid = result->gid();
					}
			        change_owner(f, st, uid, gid);
			        change_permissions(f, 0755);
					heal_entities_subdir(f, uid, gid);
				}
			}
		);
	}

	void
	heal_forms(const ggg::Hierarchy& h) {
		std::vector<sys::gid_type> form_entities;
		sys::idirtree tree(sys::path(GGG_ROOT, "reg"));
		std::for_each(
			sys::idirtree_iterator<sys::directory_entry>(tree),
			sys::idirtree_iterator<sys::directory_entry>(),
			[&h,&tree,&form_entities] (const sys::directory_entry& entry) {
			    sys::path f(tree.current_dir(), entry.name());
			    sys::file_status st(f);
			    if (st.type() == sys::file_type::regular) {
			        auto result = h.find_by_name(entry.name());
			        sys::uid_type uid = 0;
			        sys::gid_type gid = 0;
			        if (result == h.end()) {
						ggg::native_message(
							std::cerr,
							"Unable to find form entity _.",
							entry.name()
						);
					} else {
			            uid = result->id();
			            gid = result->gid();
						form_entities.push_back(gid);
					}
					change_owner(f, st, uid, gid);
			        change_permissions(f, 0600);
					sys::file_mode m = (uid == 0) ? 0 : 0600;
					make_directory(sys::path(GGG_ROOT, "acc"), entry.name());
					make_file(
						sys::path(GGG_ROOT, "acc", entry.name(), "shadow"),
						uid,
						gid,
						m,
						false
					);
					make_directory(
						sys::path(GGG_ROOT, "ent"),
						entry.name(),
						uid,
						gid,
						0755
					);
				} else if (st.type() == sys::file_type::directory) {
					change_owner(f, st, 0, 0);
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
			if (write_gid != 0) {
				acl.add_group(write_gid, permission_type::read_write);
			}
			acl.add_mask();
			change_acl(GGG_LOCK_FILE, acl, access_acl);
		}
	}

	void
	heal_accounts(const ggg::Hierarchy& h) {
		const sys::file_mode file_perms(0600), dir_perms(0700);
		sys::idirtree tree(sys::path(GGG_ROOT, "acc"));
		for (const sys::directory_entry entry : tree.entries<sys::directory_entry>()) {
		    sys::path f(tree.current_dir(), entry.name());
		    sys::file_status st(f);
			std::string name;
			sys::file_mode m;
			ggg::acl::permission_type acl_perms;
		    if (st.type() == sys::file_type::directory) {
				name = entry.name();
				m = dir_perms;
				acl_perms = ggg::acl::permission_type::read |
					ggg::acl::permission_type::execute;
			} else if (st.type() == sys::file_type::regular) {
				sys::canonical_path dir(tree.current_dir());
				name = dir.basename();
				m = file_perms;
				acl_perms = ggg::acl::permission_type::read;
			}
		    auto result = h.find_by_name(name.data());
		    sys::uid_type uid = 0;
		    sys::gid_type gid = 0;
		    if (result != h.end()) {
		        uid = result->id();
		        gid = result->gid();
			}
			if (uid == 0) {
				m = 0;
			}
			change_owner(f, st, uid, gid);
			if (!has_ggg_groups()) {
				change_permissions(f, m);
			} else {
				using namespace ggg::acl;
				access_control_list acl(m);
				if (read_gid != 0) {
					acl.add_group(read_gid, acl_perms);
				}
				if (write_gid != 0) {
					acl.add_group(write_gid, acl_perms | permission_type::write);
				}
				acl.add_mask();
				change_acl(f, acl, access_acl);
				if (st.is_directory()) {
					access_control_list acl0(file_perms);
					add_directory_acl(acl0);
					acl0.add_mask();
					change_acl(f, acl0, default_acl);
				}
			}
		}
		if (has_ggg_groups()) {
			using namespace ggg::acl;
			sys::path f(GGG_ROOT, "acc");
			{
				permission_type acl_perms =
					permission_type::read | permission_type::execute;
				access_control_list acl(sys::file_mode(0755));
				if (read_gid != 0) {
					acl.add_group(read_gid, acl_perms);
				}
				if (write_gid != 0) {
					acl.add_group(write_gid, acl_perms | permission_type::write);
				}
				acl.add_mask();
				change_acl(f, acl, access_acl);
			}
			{
				access_control_list acl(sys::file_mode(0755));
				add_directory_acl(acl);
				acl.add_mask();
				change_acl(f, acl, default_acl);
			}
		}
	}

}

void
ggg::Heal
::execute() {
	init_locale();
	make_directory("/", GGG_ROOT, 0, 0, 0755);
	make_directory(GGG_ROOT, "ent", 0, 0, 0755);
	make_directory(GGG_ROOT, "reg", 0, 0, 0755);
	make_directory(GGG_ROOT, "acc", 0, 0);
	change_owner(GGG_ROOT, sys::file_status(GGG_ROOT), 0, 0);
	change_permissions(GGG_ROOT, 0755);
	make_file(GGG_LOCK_FILE, 0, 0, lock_file_mode, false);
	ggg::Hierarchy h;
	try {
		h.open(sys::path(GGG_ROOT, "ent"));
	} catch (const std::exception& err) {
		++num_errors;
		native_sentence(std::cerr, "Entities are broken.");
		error_message(std::cerr, err);
	}
	find_group(h, GGG_READ_GROUP, read_gid);
	find_group(h, GGG_WRITE_GROUP, write_gid);
	heal_entities(h);
	heal_forms(h);
	heal_accounts(h);
	if (num_errors > 0) {
		throw quiet_error();
	}
}
