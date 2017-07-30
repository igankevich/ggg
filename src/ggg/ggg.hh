#ifndef GGG_GGG_HH
#define GGG_GGG_HH

#include <stdexcept>

#include "core/hierarchy.hh"
#include "control/account_control.hh"

namespace ggg {

	class Ggg {

		Hierarchy _hierarchy;
		Account_control _accounts;

	public:
		Ggg() = default;
		Ggg(const Ggg&) = delete;
		Ggg(Ggg&&) = delete;
		Ggg operator=(const Ggg&) = delete;

		inline explicit
		Ggg(const char* path, bool verbose):
		_hierarchy(sys::path(path)),
		_accounts()
		{ this->_accounts.verbose(verbose); }

		inline void
		open(const sys::path& root) {
			this->_hierarchy.open(root);
		}

		void
		erase(const char* user);

	};

}

#endif // GGG_GGG_HH
