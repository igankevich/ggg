#include "sanitise.hh"

#include <unistd.h>

#include <algorithm>
#include <iostream>

#include <unistdx/base/check>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/path>

#include <ggg/config.hh>
#include <ggg/core/native.hh>

namespace {

	void
	remove_empty_files(sys::path dir) {
		std::vector<sys::path> empty_files;
		sys::idirtree tree(dir);
		std::for_each(
			sys::idirtree_iterator<sys::directory_entry>(tree),
			sys::idirtree_iterator<sys::directory_entry>(),
			[&empty_files,&tree] (const sys::directory_entry& entry) {
				sys::path p(tree.current_dir(), entry.name());
				sys::file_status st(p);
				if (st.is_regular() && st.size() == 0 && st.owner() == 0) {
					empty_files.emplace_back(std::move(p));
				}
			}
		);
		for (const sys::path& p : empty_files) {
			try {
				ggg::native_message(std::clog, "Removing empty file _.", p);
				UNISTDX_CHECK(::unlink(p));
			} catch (const std::exception& err) {
				ggg::error_message(std::cerr, err);
			}
		}
	}

}

void
ggg::Sanitise
::execute() {
	remove_empty_files(sys::path(GGG_ROOT, "ent"));
}
