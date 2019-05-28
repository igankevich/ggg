#ifndef GGG_CORE_DATABASE_HH
#define GGG_CORE_DATABASE_HH

#include <ostream>
#include <string>
#include <unordered_map>

#include <ggg/core/account.hh>
#include <ggg/core/entity.hh>
#include <ggg/core/group.hh>
#include <ggg/core/host.hh>
#include <ggg/core/host_address.hh>
#include <ggg/core/ip_address.hh>
#include <ggg/core/machine.hh>

#include <sqlitex/connection.hh>
#include <sqlitex/transaction.hh>
#include <unistdx/ipc/identity>

namespace ggg {

	class Transaction;

	class Database {

	public:
		enum class File: int {
			Entities=0,
			Accounts=1,
			All=-1
		};

		enum class Flag {
			Read_only,
			Read_write
		};


	public:
		typedef sqlite::connection database_t;
		typedef sqlite::statement statement_type;
		typedef std::unordered_map<sys::gid_type,group> group_container_t;

	private:
		database_t _db;

	public:

		Database() = default;

		inline explicit
		Database(File file, Flag flag=Flag::Read_only) {
			this->open(file, flag);
		}

		inline
		~Database() {
			this->close();
		}

		void
		open(File file, Flag flag=Flag::Read_only);

		inline bool
		is_open() const noexcept {
			return this->_db.get() != nullptr;
		}

		inline void
		close() {
			this->_db.close();
		}

		inline database_t*
		db() {
			return &this->_db;
		}

		inline const database_t*
		db() const {
			return &this->_db;
		}

		statement_type
		find_entity(int64_t id);

		statement_type
		find_entity(const char* name);

		statement_type
		entities();

		statement_type
		search_entities();

		inline statement_type
		find_user(sys::uid_type uid) {
			return this->find_entity(static_cast<int64_t>(uid));
		}

		inline statement_type
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
			for (const auto& tmp : rstr.template rows<entity>()) {
				*result++ = tmp;
			}
		}

		inline statement_type
		users() {
			return this->entities();
		}

		void
		update(const entity& ent);

		bool
		find_group(sys::gid_type gid, ggg::group& result);

		bool
		find_group(const char* name, ggg::group& result);

		statement_type
		find_parent_entities(const char* name);

		statement_type
		find_child_entities(const char* name);

		group_container_t
		groups();

		statement_type
		ties();

		sys::uid_type
		next_entity_id();

		void
		insert(const entity& ent);

		void
		erase(const char* name);

		sys::uid_type
		find_id(const char* name);

		inline bool
		contains(const char* name) {
			return this->find_id_nocheck(name) != ggg::bad_uid;
		}

		std::string
		find_name(sys::uid_type id);

		void
		dot(std::ostream& out);

		statement_type
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
			for (const auto& tmp : rstr.template rows<account>()) {
				*result++ = tmp;
			}
		}

		statement_type
		accounts();

		void
		insert(const account& acc);

		void
		update(const account& acc);

		void
		set_password(const account& acc);

		void
		expire(const char* name);

		statement_type
		expired_entities();

		statement_type
		expired_ids();

		statement_type
		expired_names();

		void
		set_account_flag(const char* name, account_flags flag);

		void
		unset_account_flag(const char* name, account_flags flag);

		statement_type
		find_entities_by_flag(account_flags flag, bool set);

		inline void
		expire_password(const char* name) {
			this->set_account_flag(name, account_flags::password_has_expired);
		}

		inline void
		reset_password(const char* name) {
			this->unset_account_flag(name, account_flags::password_has_expired);
		}

		inline void
		activate(const char* name) {
			this->unset_account_flag(name, account_flags::suspended);
		}

		inline void
		deactivate(const char* name) {
			this->set_account_flag(name, account_flags::suspended);
		}

		inline statement_type
		locked_entities() {
			return this->find_entities_by_flag(account_flags::suspended, true);
		}

		statement_type
		hierarchy();

		void
		attach(const char* child, const char* parent);

		void
		attach(sys::uid_type child_id, sys::gid_type parent_id);

		void
		detach(const char* child);

		bool
		entities_are_attached(sys::uid_type child_id, sys::gid_type parent_id);

		void
		tie(sys::uid_type uid, sys::gid_type gid);

		void
		tie(const char* child, const char* parent);

		void
		untie(const char* child, const char* parent);

		void
		untie(const char* child);

		bool
		entities_are_tied(sys::uid_type child_id, sys::gid_type parent_id);

		sys::uid_type
		find_hierarchy_root(sys::uid_type child_id);

		statement_type hosts();
		statement_type find_host(const char* name);
		statement_type find_host(const sys::ethernet_address& address);
		statement_type host_addresses();
		statement_type find_ip_address(const char* name, sys::family_type family);
		statement_type find_ip_address(const sys::ethernet_address& hwaddr);
		statement_type find_host_name(const ip_address& address);
		statement_type machines();
		void insert(const Machine& rhs);
		void remove(const Machine& rhs);
		void remove_all_machines();

	private:

		std::string
		select_users_by_names(int n);

		std::string
		select_accounts_by_names(int n);

		void
		attach(File file, Flag flag=Flag::Read_only);

		void
		validate_entity(const entity& ent);

		sys::uid_type
		find_id_nocheck(const char* name);

		std::string
		find_name_nocheck(sys::uid_type id);

		friend class Transaction;

	};

	typedef sqlite::row_iterator<ggg::entity> user_iterator;
	typedef sqlite::row_iterator<ggg::group> group_iterator;
	typedef sqlite::row_iterator<ggg::account> account_iterator;
	typedef sqlite::row_iterator<ggg::host> host_iterator;
	typedef sqlite::row_iterator<ggg::Machine> machine_iterator;

	class Transaction: public sqlite::immediate_transaction {

	public:

		inline explicit
		Transaction(Database& db):
		sqlite::immediate_transaction(db._db) {}

	};

}

#endif // vim:filetype=cpp
