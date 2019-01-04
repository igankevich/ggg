#ifndef GGG_GGG_HH
#define GGG_GGG_HH

#include <algorithm>
#include <set>
#include <string>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
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
		Database _database;
		bool _verbose = true;

	public:
		basic_ggg() = default;

		basic_ggg(const basic_ggg&) = delete;

		basic_ggg(basic_ggg&&) = delete;

		basic_ggg
		operator=(const basic_ggg&) = delete;

		inline explicit
		basic_ggg(const char* path, bool verbose):
		_database(GGG_DATABASE_PATH),
		_verbose(verbose) {
			this->init();
		}

		inline void
		open(const sys::path& root) {
			this->_database.open(GGG_DATABASE_PATH),
			this->init();
		}

		inline sqlite::rstream
		find_user(sys::uid_type uid) {
			return this->_database.find_user(uid);
		}

		inline void
		erase(const string_type& user) {
			this->_database.erase(user.data());
		}

		inline void
		expire(const std::string& user) {
			this->_database.expire(user.data());
		}

		inline void
		reset(const std::string& user) {
			this->_database.expire_password(user.data());
		}

		inline void
		activate(const std::string& user) {
			this->_database.activate(user.data());
		}

		template <class Iterator, class Result>
		inline void
		find_entities(Iterator first, Iterator last, Result result) {
			this->_database.find_users(first, last, result);
		}

		template <class Container, class Result>
		inline void
		find_accounts(const Container& names, Result result) {
			this->_database.find_accounts(names.begin(), names.end(), result);
		}

		inline bool
		contains(const string_type& name) {
			return this->_database.find_id(name.data()) != bad_uid;
		}

		entity_type
		generate(const string_type& name);

		std::set<entity_type>
		generate(const std::unordered_set<string_type>& names);

		inline void
		update(const entity_type& ent) {
			this->_database.update(ent);
		}

		inline void
		update(const account& acc) {
			this->_database.update(acc);
		}

		inline void
		add(const entity_type& ent) {
			this->_database.insert(ent);
		}

		inline bool
		verbose() const noexcept {
			return this->_verbose;
		}

		inline sys::gid_type
		find_auth_group() {
			sys::gid_type gid = this->_database.find_id(GGG_READ_GROUP);
			if (gid == bad_gid) {
				gid = 0;
			}
			return gid;
		}

		inline sys::gid_type
		find_write_group() {
			sys::gid_type gid = this->_database.find_id(GGG_WRITE_GROUP);
			if (gid == bad_gid) {
				gid = 0;
			}
			return gid;
		}

	private:

		inline void
		init() {
//			this->_accounts.set_read_group(this->find_auth_group());
//			this->_accounts.set_write_group(this->find_write_group());
		}

	};

	typedef basic_ggg<char> GGG;

}

#endif // GGG_GGG_HH
