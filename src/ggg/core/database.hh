#ifndef GGG_CORE_DATABASE_HH
#define GGG_CORE_DATABASE_HH

#include <ostream>
#include <unordered_map>

#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/group.hh>
#include <sqlitex/database.hh>
#include <unistdx/ipc/identity>

namespace ggg {

	class Database {

	public:
		typedef sqlite::database database_t;
		typedef sqlite::rstream row_stream_t;
		typedef std::unordered_map<sys::gid_type,group> group_container_t;

	private:
		bool _readonly = true;
		database_t _db;

	public:

		Database() = default;

		inline explicit
		Database(const char* filename, bool read=true) {
			this->open(filename, read);
		}

		inline
		~Database() {
			// TODO this is neede only in NSS module
			try {
				this->close();
			} catch (...) {
				// ignore errors for NSS modules
			}
		}

		void
		open(const char* filename, bool read=true);

		inline bool
		is_open() const noexcept {
			return this->_db.db() != nullptr;
		}

		inline void
		close() {
			this->_db.close();
		}

		row_stream_t
		find_entity(int64_t id);

		row_stream_t
		find_entity(const char* name);

		row_stream_t
		entities();

		inline row_stream_t
		find_user(sys::uid_type uid) {
			return this->find_entity(static_cast<int64_t>(uid));
		}

		inline row_stream_t
		find_user(const char* name) {
			return this->find_entity(name);
		}

		template <class Iterator, class Result>
		inline void
		find_users(Iterator first, Iterator last, Result result) {
			auto n = std::distance(first, last);
			auto rstr = this->_db.prepare(
				this->select_users_by_names(n).data()
			);
			int i = 0;
			while (first != last) {
				rstr.bind(++i, *first);
				++first;
			}
			entity tmp;
			while (rstr >> tmp) {
				*result++ = tmp;
			}
		}

		inline row_stream_t
		users() {
			return this->entities();
		}

		void
		update(const entity& ent);

		bool
		find_group(sys::gid_type gid, ggg::group& result);

		bool
		find_group(const char* name, ggg::group& result);

		row_stream_t
		find_parent_entities(const char* name);

		row_stream_t
		find_child_entities(const char* name);

		group_container_t
		groups();

		row_stream_t
		ties();

		void
		insert(const entity& ent);

		void
		erase(const char* name);

		void
		tie(sys::uid_type uid, sys::gid_type gid);

		sys::uid_type
		find_id(const char* name);

		void
		dot(std::ostream& out);

		row_stream_t
		find_account(const char* name);

		template <class Iterator, class Result>
		inline void
		find_accounts(Iterator first, Iterator last, Result result) {
			auto n = std::distance(first, last);
			auto rstr = this->_db.prepare(
				this->select_accounts_by_names(n).data()
			);
			int i = 0;
			while (first != last) {
				rstr.bind(++i, *first);
				++first;
			}
			account tmp;
			while (rstr >> tmp) {
				*result++ = tmp;
			}
		}

		row_stream_t
		accounts();

		void
		update(const account& acc);

		void
		set_password(const account& acc);

		void
		expire(const char* name);

		row_stream_t
		expired_entities();

		row_stream_t
		expired_ids();

		row_stream_t
		expired_names();

		void
		set_flag(const char* name, account_flags flag);

		void
		unset_flag(const char* name, account_flags flag);

		row_stream_t
		find_entities_by_flag(account_flags flag, bool set);

		inline void
		expire_password(const char* name) {
			this->set_flag(name, account_flags::password_has_expired);
		}

		inline void
		reset_password(const char* name) {
			this->unset_flag(name, account_flags::password_has_expired);
		}

		inline void
		activate(const char* name) {
			this->unset_flag(name, account_flags::suspended);
		}

		inline void
		deactivate(const char* name) {
			this->set_flag(name, account_flags::suspended);
		}

		inline row_stream_t
		locked_entities() {
			return this->find_entities_by_flag(account_flags::suspended, true);
		}

	private:

		std::string
		select_users_by_names(int n);

		std::string
		select_accounts_by_names(int n);

	};

	typedef sqlite::rstream_iterator<ggg::entity> user_iterator;
	typedef sqlite::rstream_iterator<ggg::group> group_iterator;
	typedef sqlite::rstream_iterator<ggg::account> account_iterator;

}

#endif // vim:filetype=cpp
