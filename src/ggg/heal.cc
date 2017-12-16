#include "heal.hh"

#include <fcntl.h>

#include <iostream>

#include <unistdx/fs/canonical_path>
#include <unistdx/fs/file_stat>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/mkdirs>
#include <unistdx/fs/path>
#include <unistdx/io/fildes>

#include "config.hh"

namespace {

	void
	make_directory(const char* root, std::string name) {
		sys::path p(root, name);
		sys::file_stat st(p);
		if (!st.exists()) {
			try {
				name += '/';
				std::clog << "creating " << p << std::endl;
				sys::mkdirs(sys::path(root), sys::path(name));
			} catch (const std::exception& err) {
				std::cerr
				    << "failed to create "
				    << p << ": " << err.what()
				    << std::endl;
			}
		}
	}

	void
	change_permissions(const char* filename, sys::file_mode m) {
		try {
			sys::file_stat st(filename);
			if (st.exists() && st.mode() != m) {
				std::clog
				    << "changing permissions of "
				    << filename << " to " << m << std::endl;
				UNISTDX_CHECK(::chmod(filename, m));
			}
		} catch (const std::exception& err) {
			std::cerr
			    << "failed to change permissions of "
			    << filename << " to " << m << ": " << err.what()
			    << std::endl;
		}
	}

	void
	change_owner(const char* filename, sys::uid_type uid, sys::gid_type gid) {
		try {
			UNISTDX_CHECK(::chown(filename, uid, gid));
		} catch (const std::exception& err) {
			std::cerr
			    << "failed to change owner of "
			    << filename << " to " << uid << ':' << gid << ": " << err.what()
			    << std::endl;
		}
	}

	void
	make_file(const char* filename, sys::file_mode m) {
		sys::fildes tmp(
			filename,
			sys::open_flag::create | sys::open_flag::read_only,
			m
		);
	}

}

void
ggg::Heal::execute() {
	make_directory(GGG_ROOT, "ent");
	make_directory(GGG_ROOT, "reg");
	make_directory(GGG_ROOT, "acc");
	change_permissions(GGG_ROOT, 0755);
	change_permissions(GGG_LOCK_FILE, 0644);
	sys::idirtree tree(sys::path(GGG_ROOT));
	std::for_each(
		sys::idirtree_iterator<sys::pathentry>(tree),
		sys::idirtree_iterator<sys::pathentry>(),
		[] (const sys::pathentry& entry) {
			sys::path f(entry.getpath());
			if (f == GGG_SHADOW) {
				return;
			}
			sys::file_stat st(f);
			if (st.owner() != 0 || st.group() != 0) {
				change_owner(f, 0, 0);
			}
			if (st.type() == sys::file_type::regular) {
				change_permissions(f, 0644);
			} else if (st.type() == sys::file_type::directory) {
				change_permissions(f, 0755);
			}
		}
	);
	make_file(GGG_SHADOW, 0);
	change_permissions(GGG_SHADOW, 0);
	change_owner(GGG_SHADOW, 0, 0);
}
