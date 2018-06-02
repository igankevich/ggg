#ifndef GGG_CORE_IACCTREE_HH
#define GGG_CORE_IACCTREE_HH

#include <unistdx/fs/idirtree>

#include <ggg/core/fs.hh>

namespace ggg {

	struct ignore_hidden_and_inaccessible_files:
		public sys::ignore_hidden_files {

		inline bool
		operator()(const sys::path& prefix, const sys::direntry& rhs) const {
			return sys::ignore_hidden_files::operator()(prefix, rhs) &&
			       can_read(sys::path(prefix, rhs.name()));
		}

		inline bool
		operator()(const sys::path& prefix, const sys::pathentry& rhs) const {
			return sys::ignore_hidden_files::operator()(prefix, rhs) &&
			       can_read(rhs.getpath());
		}

	};

	struct ignore_hidden_and_inaccessible_dirs: public sys::ignore_hidden_dirs {

		inline bool
		operator()(const sys::path& prefix, const sys::direntry& rhs) {
			return sys::ignore_hidden_dirs::operator()(prefix, rhs) &&
			       can_read(sys::path(prefix, rhs.name()));
		}

		inline bool
		operator()(const sys::path& prefix, const sys::pathentry& rhs) {
			return sys::ignore_hidden_dirs::operator()(prefix, rhs) &&
			       can_read(rhs.getpath());
		}

	};

	typedef sys::basic_idirtree<ignore_hidden_and_inaccessible_files,
	                            ignore_hidden_and_inaccessible_dirs> iacctree;
	template<class T>
	using iacctree_iterator = sys::basic_istream_iterator<iacctree, T>;

}

#endif // vim:filetype=cpp
