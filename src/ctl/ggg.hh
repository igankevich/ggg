#ifndef GGG_GGG_HH
#define GGG_GGG_HH

#include <string>
#include <algorithm>

#include "core/hierarchy.hh"
#include "ctl/account_ctl.hh"

namespace ggg {

	class GGG {

		Hierarchy _hierarchy;
		account_ctl _accounts;
		bool _verbose = true;

	public:
		GGG() = default;
		GGG(const GGG&) = delete;
		GGG(GGG&&) = delete;
		GGG operator=(const GGG&) = delete;

		inline explicit
		GGG(const char* path, bool verbose):
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

		template <class Iterator, class Result>
		inline void
		find_entities(Iterator first, Iterator last, Result result) {
			std::for_each(
				first, last,
				[&] (const std::string& rhs) {
					auto it = this->_hierarchy.find_by_name(rhs.data());
					if (it != this->_hierarchy.end()) {
						*result++ = *it;
					}
				}
			);
		}

		template <class Container, class Result>
		inline void
		find_accounts(const Container& names, Result result) {
			this->_accounts.for_each(
				[&] (const account& rhs) {
					std::string key(rhs.login().begin(), rhs.login().end());
					auto it = names.find(key);
					if (it != names.end()) {
						*result++ = rhs;
					}
				}
			);
		}

		bool
		contains(const std::string& name);

		entity
		generate(const std::string& name);

		inline sys::uid_type
		next_uid() const {
			return this->_hierarchy.next_uid();
		}

		void
		update(const entity& ent);

		void
		update(const account& ent);

		void
		add(const entity& ent, const std::string& filename);

		inline bool
		verbose() const noexcept {
			return this->_verbose;
		}


	private:

		std::string
		to_relative_path(const std::string& filename);

		void
		mkdirs(std::string relative_path);

	};

}

#endif // GGG_GGG_HH
