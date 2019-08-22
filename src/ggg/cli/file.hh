#ifndef GGG_CLI_FILE_HH
#define GGG_CLI_FILE_HH

#include <unistdx/base/check>
#include <unistdx/fs/file_mode>
#include <unistdx/ipc/identity>

namespace ggg {

	struct File {

		const char* filename;

		inline void
		owner(sys::uid_type uid, sys::gid_type gid) {
			UNISTDX_CHECK(::chown(filename, uid, gid));
		}

		inline void
		mode(sys::file_mode m) {
			UNISTDX_CHECK(::chmod(filename, m));
		}

	};

	struct Directory: public File {

		inline explicit
		Directory(const char* name): File{name} {}

		inline void
		make(sys::file_mode m) {
			int ret = ::mkdir(filename, m);
			if (ret != 0 && errno != EEXIST) {
				UNISTDX_CHECK(ret);
			}
		}

	};

}

#endif // vim:filetype=cpp
