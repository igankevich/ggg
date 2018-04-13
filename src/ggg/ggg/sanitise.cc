#include "sanitise.hh"

#include <unistd.h>

#include <algorithm>
#include <iostream>

#include <unistdx/base/check>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/path>
#include <unistdx/fs/pathentry>

#include <ggg/config.hh>
#include <ggg/core/native.hh>

namespace {

	void
	remove_empty_files(sys::path dir) {
		std::vector<sys::path> empty_files;
		sys::idirtree tree(dir);
		std::for_each(
			sys::idirtree_iterator<sys::pathentry>(tree),
			sys::idirtree_iterator<sys::pathentry>(),
			[&empty_files] (const sys::pathentry& entry) {
				sys::file_stat st(entry.getpath());
				if (st.is_regular() && st.size() == 0 && st.owner() == 0) {
					empty_files.emplace_back(entry.getpath());
				}
			}
		);
		ggg::bits::wcvt_type cv;
		for (const sys::path& p : empty_files) {
			try {
				ggg::native_message(std::wclog, "Removing empty file _.", cv.from_bytes(p));
				UNISTDX_CHECK(::unlink(p));
			} catch (const std::exception& err) {
				ggg::error_message(std::wcerr, cv, err);
			}
		}
	}

}

void
ggg::Sanitise
::execute() {
	remove_empty_files(sys::path(GGG_ROOT, "ent"));
}
