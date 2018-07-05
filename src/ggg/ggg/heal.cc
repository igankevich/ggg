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
#include <ggg/core/native.hh>
#include <ggg/ggg/quiet_error.hh>
#include <ggg/ggg/access_control_list.hh>

namespace {

	size_t num_errors = 0;
	ggg::bits::wcvt_type cv;

	const sys::file_mode lock_file_mode(0644);
	sys::gid_type auth_gid = 0;

	void
	change_permissions_priv(
		const char* filename,
		const sys::file_status& st,
		sys::file_mode m
	) {
		try {
			if (st.mode() != m) {
				ggg::native_message(
					std::wclog,
					"Changing permissions of _ to _.",
					filename,
					m
				);
				UNISTDX_CHECK(::chmod(filename, m));
			}
		} catch (const std::exception& err) {
			++num_errors;
			ggg::native_sentence(
				std::wcerr,
				cv,
				"Failed to change permissions of _ to _. ",
				cv.from_bytes(filename),
				m
			);
			ggg::error_message(std::wcerr, cv, err);
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
				std::wcerr,
				cv,
				"Failed to change permissions of _ to _. ",
				cv.from_bytes(filename),
				m
			);
			ggg::error_message(std::wcerr, cv, err);
		}
	}

	void
	change_permissions(const char* filename, sys::file_mode m) {
		try {
			change_permissions_priv(filename, m);
		} catch (const sys::bad_call& err) {
			++num_errors;
			ggg::native_sentence(
				std::wcerr,
				cv,
				"Failed to change permissions of _ to _. ",
				cv.from_bytes(filename),
				m
			);
			ggg::error_message(std::wcerr, cv, err);
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
					std::wclog,
					"Changing owner of _ to _:_.",
					filename,
					uid,
					gid
				);
				UNISTDX_CHECK(::chown(filename, uid, gid));
			} catch (const std::exception& err) {
				++num_errors;
				ggg::native_sentence(
					std::wcerr,
					cv,
					"Failed to change owner of _ to _:_. ",
					cv.from_bytes(filename),
					uid,
					gid
				);
				ggg::error_message(std::wcerr, cv, err);
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
			acl.write(filename, tp);
		}
	}

	void
	change_acl(
		const char* filename,
		ggg::acl::access_control_list& acl,
		ggg::acl::acl_type tp
	) {
		ggg::native_message(std::wclog, "Changing ACLs of _.", filename);
		try {
			change_acl_priv(filename, acl, tp);
		} catch (const std::exception& err) {
			++num_errors;
			ggg::native_sentence(
				std::wcerr,
				cv,
				"Failed to change ACLs of _. ",
				cv.from_bytes(filename)
			);
			ggg::error_message(std::wcerr, cv, err);
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
			ggg::native_message(
				std::wcerr,
				"_ is not a regular file.",
				cv.from_bytes(filename)
			);
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
			ggg::native_sentence(
				std::wcerr,
				cv,
				"Failed to create _. ",
				cv.from_bytes(filename)
			);
			ggg::error_message(std::wcerr, cv, err);
		}
	}

	void
	make_directory_priv(
		const char* root,
		std::string name,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m,
		bool change_mode_and_owner = true
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
			ggg::native_message(std::wclog, "Creating _.", cv.from_bytes(p));
			sys::mkdirs(p);
			st.update(p);
		}
		if (!st.is_directory()) {
			ggg::native_message(
				std::wcerr,
				"_ is not a directory.",
				cv.from_bytes(p)
			);
		} else if (change_mode_and_owner) {
			change_owner(p, st, uid, gid);
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
		bool change_mode_and_owner = true
	) {
		try {
			make_directory_priv(root, name, uid, gid, m, change_mode_and_owner);
		} catch (const sys::bad_call& err) {
			sys::path p(root, name);
			++num_errors;
			ggg::native_sentence(
				std::wcerr,
				cv,
				"Failed to create _. ",
				cv.from_bytes(p)
			);
			ggg::error_message(std::wcerr, cv, err);
		}
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
							std::wcerr,
							"Unable to find form entity _.",
							cv.from_bytes(entry.name())
						);
					} else {
			            uid = result->id();
			            gid = result->gid();
						form_entities.push_back(gid);
					}
					change_owner(f, st, uid, gid);
			        change_permissions(f, 0600);
					sys::file_mode m = (uid == 0) ? 0 : 0600;
					make_directory(
						sys::path(GGG_ROOT, "acc"),
						entry.name(),
						uid,
						gid,
						0700,
						false
					);
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
			acl.add_mask();
			change_acl(GGG_LOCK_FILE, acl, access_acl);
		}
	}

	void
	heal_accounts(const ggg::Hierarchy& h) {
		sys::idirtree tree(sys::path(GGG_ROOT, "acc"));
		std::for_each(
			sys::idirtree_iterator<sys::directory_entry>(tree),
			sys::idirtree_iterator<sys::directory_entry>(),
			[&h,&tree] (const sys::directory_entry& entry) {
			    sys::path f(tree.current_dir(), entry.name());
			    sys::file_status st(f);
				std::string name;
				sys::file_mode m;
			    if (st.type() == sys::file_type::directory) {
					name = entry.name();
					m = 0700;
				} else if (st.type() == sys::file_type::regular) {
					sys::canonical_path dir(tree.current_dir());
					name = dir.basename();
					m = 0600;
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
				if (auth_gid == 0) {
					change_permissions(f, m);
				} else {
					using namespace ggg::acl;
					access_control_list acl(m);
					acl.add_group(auth_gid, permission_type::read);
					acl.add_mask();
					change_acl(f, acl, access_acl);
					if (st.is_directory()) {
						change_acl(f, acl, default_acl);
					}
				}
			}
		);
	}

}

void
ggg::Heal
::execute() {
	init_locale();
	make_directory("/", GGG_ROOT, 0, 0, 0755);
	make_directory(GGG_ROOT, "ent", 0, 0, 0755);
	make_directory(GGG_ROOT, "reg", 0, 0, 0755);
	make_directory(GGG_ROOT, "acc", 0, 0, 0755);
	change_owner(GGG_ROOT, sys::file_status(GGG_ROOT), 0, 0);
	change_permissions(GGG_ROOT, 0755);
	make_file(GGG_LOCK_FILE, 0, 0, lock_file_mode, false);
	ggg::Hierarchy h;
	try {
		h.open(sys::path(GGG_ROOT, "ent"));
	} catch (const std::exception& err) {
		++num_errors;
		native_sentence(std::wcerr, cv, "Entities are broken.");
		error_message(std::wcerr, cv, err);
	}
	auto result = h.find_by_name(GGG_AUTH_GROUP);
	if (result != h.end()) {
	    auth_gid = result->gid();
	}
	heal_entities(h);
	heal_forms(h);
	heal_accounts(h);
	if (num_errors > 0) {
		throw quiet_error();
	}
}
