#ifndef GGG_GGG_HH
#define GGG_GGG_HH

#include <algorithm>
#include <set>
#include <string>

#include <ggg/core/hierarchy.hh>
#include <ggg/ctl/account_ctl.hh>

namespace ggg {

	template <class Ch>
	class basic_ggg {

	public:
		typedef Ch char_type;
		typedef std::basic_string<Ch> string_type;
		typedef basic_hierarchy<Ch> hierarchy_type;
		typedef basic_entity<Ch> entity_type;

	private:
		hierarchy_type _hierarchy;
		account_ctl _accounts;
		bool _verbose = true;

	public:
		basic_ggg() = default;

		basic_ggg(const basic_ggg&) = delete;

		basic_ggg(basic_ggg&&) = delete;

		basic_ggg
		operator=(const basic_ggg&) = delete;

		inline explicit
		basic_ggg(const char* path, bool verbose):
		_hierarchy(path),
		_accounts(),
		_verbose(verbose) {
			this->_hierarchy.verbose(verbose);
			this->_accounts.verbose(verbose);
		}

		inline void
		open(const sys::path& root) {
			this->_hierarchy.open(root);
		}

		void
		erase(const string_type& user);

		inline void
		expire(const std::string& user) {
			this->_accounts.expire(user.data());
		}

		inline void
		reset(const std::string& user) {
			this->_accounts.expire_password(user.data());
		}

		inline void
		activate(const std::string& user) {
			this->_accounts.activate(user.data());
		}

		template <class Iterator, class Result>
		inline void
		find_entities(Iterator first, Iterator last, Result result) {
			std::for_each(
				first,
				last,
				[&] (const string_type& rhs) {
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
			auto end = this->_accounts.end();
			for (const auto& n : names) {
				auto result2 = this->_accounts.find(n.data());
				if (result2 != end) {
					*result++ = *result2;
				}
			}
		}

		bool
		contains(const string_type& name);

		entity_type
		generate(const string_type& name);

		std::set<entity_type>
		generate(const std::unordered_set<string_type>& names);

		inline sys::uid_type
		next_uid() const {
			return this->_hierarchy.next_uid();
		}

		inline void
		update(const entity_type& ent) {
			this->_hierarchy.update(ent);
		}

		inline void
		update(const account& acc) {
			this->_accounts.update(acc);
		}

		void
		add(const entity_type& ent, const account& acc);

		inline bool
		verbose() const noexcept {
			return this->_verbose;
		}

		inline const hierarchy_type&
		hierarchy() const noexcept {
			return this->_hierarchy;
		}

		inline const account_ctl&
		accounts() const noexcept {
			return this->_accounts;
		}

		sys::path
		account_origin(sys::uid_type uid);

	private:

		std::string
		to_relative_path(const std::string& filename);

		void
		mkdirs(std::string relative_path);

	};

	typedef basic_ggg<char> GGG;
	typedef basic_ggg<wchar_t> WGGG;

}

#endif // GGG_GGG_HH
