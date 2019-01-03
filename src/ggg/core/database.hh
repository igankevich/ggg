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

		inline void
		flush() {
			sqlite3_db_cacheflush(_db.db());
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

		inline row_stream_t
		users() {
			return this->entities();
		}

		bool
		find_group(sys::gid_type gid, ggg::group& result);

		bool
		find_group(const char* name, ggg::group& result);

		group_container_t
		groups();

		row_stream_t
		ties();

		void
		insert(const entity& ent);

		void
		tie(sys::uid_type uid, sys::gid_type gid);

		sys::uid_type
		find_id(const char* name);

		void
		dot(std::ostream& out);

		row_stream_t
		find_account(const char* name);

		row_stream_t
		accounts();

		void
		update(const account& acc);

	};

}

#endif // vim:filetype=cpp
