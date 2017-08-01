#ifndef GGG_GGG_HH
#define GGG_GGG_HH

#include <string>

#include "core/hierarchy.hh"
#include "ctl/account_ctl.hh"

namespace ggg {

	class Ggg {

		Hierarchy _hierarchy;
		account_ctl _accounts;
		bool _verbose = true;

	public:
		Ggg() = default;
		Ggg(const Ggg&) = delete;
		Ggg(Ggg&&) = delete;
		Ggg operator=(const Ggg&) = delete;

		inline explicit
		Ggg(const char* path, bool verbose):
		_hierarchy(sys::path(path)),
		_accounts(),
		_verbose(verbose)
		{
			this->_hierarchy.verbose(verbose);
			this->_accounts.verbose(verbose);
		}

		inline void
		open(const sys::path& root) {
			this->_hierarchy.open(root);
		}

		void
		erase(const std::string& user);

		void
		expire(const std::string& user);

		void
		activate(const std::string& user);

		inline bool
		verbose() const noexcept {
			return this->_verbose;
		}

	};

}

#endif // GGG_GGG_HH
