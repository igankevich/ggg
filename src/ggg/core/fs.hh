#ifndef GGG_CORE_FS_HH
#define GGG_CORE_FS_HH

#include <unistdx/fs/path>

namespace ggg {

	inline bool
	can_read(const sys::path& path) {
		return ::eaccess(path, R_OK) != -1;
	}

	inline bool
	can_write(const sys::path& path) {
		return ::eaccess(path, W_OK) != -1;
	}

}

#endif // vim:filetype=cpp
