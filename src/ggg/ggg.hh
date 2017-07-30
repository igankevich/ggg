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
		Ggg(sys::path root):
		_hierarchy(root),
		_accounts()
		{}

		inline void
		open(const sys::path& root) {
			this->_hierarchy.open(root);
		}

		inline void
		erase(const char* user) {
			if (!user) {
				throw std::invalid_argument("bad user");
			}
			this->_accounts.erase(user);
		}

	};

}

#endif // GGG_GGG_HH
