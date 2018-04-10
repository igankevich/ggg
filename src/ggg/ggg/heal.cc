#include "heal.hh"

#include <fcntl.h>

#include <iostream>

#include <unistdx/fs/canonical_path>
#include <unistdx/fs/file_stat>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/mkdirs>
#include <unistdx/fs/path>
#include <unistdx/io/fildes>

#include <ggg/config.hh>

namespace {

	size_t num_errors = 0;

	void
	change_permissions(
		const char* filename,
		const sys::file_stat& st,
		sys::file_mode m
	) {
		try {
			if (st.exists() && st.mode() != m) {
				std::clog
				    << "changing permissions of "
				    << filename << " to " << m << std::endl;
				UNISTDX_CHECK(::chmod(filename, m));
			}
		} catch (const std::exception& err) {
			++num_errors;
			std::cerr
			    << "failed to change permissions of "
			    << filename << " to " << m << ": " << err.what()
			    << std::endl;
		}
	}

	void
	change_permissions(const char* filename, sys::file_mode m) {
		sys::file_stat st(filename);
		change_permissions(filename, st, m);
	}

	void
	change_owner(const char* filename, sys::uid_type uid, sys::gid_type gid) {
		try {
			std::clog
			    << "changing owner of "
			    << filename << " to " << uid << ':' << gid
				<< std::endl;
			UNISTDX_CHECK(::chown(filename, uid, gid));
		} catch (const std::exception& err) {
			++num_errors;
			std::cerr
			    << "failed to change owner of "
			    << filename << " to " << uid << ':' << gid << ": " << err.what()
			    << std::endl;
		}
	}

	void
	make_file(
		const char* filename,
		sys::uid_type uid,
		sys::gid_type gid,
		sys::file_mode m
	) {
		sys::fildes tmp(
			filename,
			sys::open_flag::create | sys::open_flag::read_only,
			m
		);
		sys::file_stat st(filename);
		if (st.exists()) {
			if (!st.is_regular()) {
				std::cerr << filename << " is not a regular file" << std::endl;
			} else {
				if (st.owner() != uid || st.group() != gid) {
					change_owner(filename, uid, gid);
				}
				change_permissions(filename, st, m);
			}
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
		sys::path p(root, name);
		sys::file_stat st(p);
		if (!st.exists()) {
			try {
				name += '/';
				std::clog << "creating " << p << std::endl;
				sys::mkdirs(sys::path(root), sys::path(name));
				st.update(p);
			} catch (const std::exception& err) {
				++num_errors;
				std::cerr
				    << "failed to create "
				    << p << ": " << err.what()
				    << std::endl;
			}
		}
		if (st.exists()) {
			if (!st.is_directory()) {
				std::cerr << p << " is not a directory" << std::endl;
			} else {
				if (st.owner() != uid || st.group() != gid) {
					change_owner(p, uid, gid);
				}
				change_permissions(p, st, m);
			}
		}
	}

	void
	heal_entities_subdir(sys::path dir, sys::uid_type uid, sys::gid_type gid) {
		sys::idirtree tree(sys::path(GGG_ROOT, "ent"));
		std::for_each(
			sys::idirtree_iterator<sys::pathentry>(tree),
			sys::idirtree_iterator<sys::pathentry>(),
			[uid,gid] (const sys::pathentry& entry) {
			    sys::path f(entry.getpath());
			    sys::file_stat st(f);
			    if (st.owner() != uid || st.group() != gid) {
			        change_owner(f, uid, gid);
				}
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
			sys::idirectory_iterator<sys::pathentry>(dir),
			sys::idirectory_iterator<sys::pathentry>(),
			[&h] (const sys::pathentry& entry) {
			    sys::path f(entry.getpath());
			    sys::file_stat st(f);
			    if (st.type() == sys::file_type::regular) {
			    	if (st.owner() != 0 || st.group() != 0) {
			    	    change_owner(f, 0, 0);
					}
			        change_permissions(f, 0644);
				} else if (st.type() == sys::file_type::directory) {
			        auto result = h.find_by_name(entry.name());
			        sys::uid_type uid = 0;
			        sys::gid_type gid = 0;
			        if (result != h.end()) {
			            uid = result->id();
			            gid = result->gid();
					}
			        if (st.owner() != uid || st.group() != gid) {
			            change_owner(f, uid, gid);
					}
			        change_permissions(f, 0755);
					heal_entities_subdir(f, uid, gid);
				}
			}
		);
	}

	void
	heal_forms(const ggg::Hierarchy& h) {
		sys::idirtree tree(sys::path(GGG_ROOT, "reg"));
		std::for_each(
			sys::idirtree_iterator<sys::pathentry>(tree),
			sys::idirtree_iterator<sys::pathentry>(),
			[&h] (const sys::pathentry& entry) {
			    sys::path f(entry.getpath());
			    sys::file_stat st(f);
			    if (st.type() == sys::file_type::regular) {
			        auto result = h.find_by_name(entry.name());
			        sys::uid_type uid = 0;
			        sys::gid_type gid = 0;
			        if (result == h.end()) {
			            std::cerr << "unable to find "
			                      << entry.name() << " form entity"
			                      << std::endl;
					} else {
			            uid = result->id();
			            gid = result->gid();
					}
			        if (st.owner() != uid || st.group() != gid) {
			            change_owner(f, uid, gid);
					}
			        change_permissions(f, 0600);
					sys::file_mode m = (uid == 0) ? 0 : 0600;
					make_directory(
						sys::path(GGG_ROOT, "acc"),
						entry.name(),
						uid,
						gid,
						0700
					);
					make_file(
						sys::path(GGG_ROOT, "acc", entry.name(), "shadow"),
						uid,
						gid,
						m
					);
					make_file(
						sys::path(GGG_ROOT, "lck", entry.name()),
						uid,
						gid,
						m
					);
					make_directory(
						sys::path(GGG_ROOT, "ent"),
						entry.name(),
						uid,
						gid,
						0755
					);
				} else if (st.type() == sys::file_type::directory) {
			        if (st.owner() != 0 || st.group() != 0) {
			            change_owner(f, 0, 0);
					}
			        change_permissions(f, 0755);
				}
			}
		);
	}

	void
	heal_accounts(const ggg::Hierarchy& h) {
		sys::idirtree tree(sys::path(GGG_ROOT, "acc"));
		std::for_each(
			sys::idirtree_iterator<sys::pathentry>(tree),
			sys::idirtree_iterator<sys::pathentry>(),
			[&h] (const sys::pathentry& entry) {
			    sys::path f(entry.getpath());
			    sys::file_stat st(f);
				std::string name;
				sys::file_mode m;
			    if (st.type() == sys::file_type::directory) {
					name = entry.name();
					m = 0700;
				} else if (st.type() == sys::file_type::regular) {
					sys::canonical_path dir(entry.dirname());
					name = dir.basename();
					m = 0600;
				}
			    auto result = h.find_by_name(name.data());
			    sys::uid_type uid = 0;
			    sys::uid_type gid = 0;
			    if (result != h.end()) {
			        uid = result->id();
			        gid = result->gid();
				}
				if (uid == 0) {
					m = 0;
				}
			    if (st.owner() != uid || st.group() != gid) {
			        change_owner(f, uid, gid);
				}
			    change_permissions(f, m);
			}
		);
	}

}

void
ggg::Heal
::execute() {
	make_directory("/", GGG_ROOT, 0, 0, 0755);
	make_directory(GGG_ROOT, "ent", 0, 0, 0755);
	make_directory(GGG_ROOT, "reg", 0, 0, 0755);
	make_directory(GGG_ROOT, "acc", 0, 0, 0755);
	make_directory(GGG_ROOT, "lck", 0, 0, 01777);
	change_owner(GGG_ROOT, 0, 0);
	change_permissions(GGG_ROOT, 0755);
	make_file(GGG_LOCK_FILE, 0, 0, 0644);
	ggg::Hierarchy h;
	try {
		h.open(GGG_ROOT);
	} catch (const std::exception& err) {
		++num_errors;
		std::cerr
			<< "unable to fully " << this->prefix()
			<< " accounts and forms: entities are broken"
			<< std::endl;
	}
	heal_entities(h);
	heal_forms(h);
	heal_accounts(h);
	if (num_errors > 0) {
		std::stringstream msg;
		msg << "unable to fully " << this->prefix() << " directories";
		throw std::runtime_error(msg.str());
	}
}
