#ifndef GGG_CORE_DATABASE_HH
#define GGG_CORE_DATABASE_HH

#include <unistd.h>

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
#include <ggg/core/message.hh>
#include <ggg/core/public_key.hh>
#include <ggg/core/ties.hh>

#include <sqlitex/connection.hh>
#include <sqlitex/transaction.hh>
#include <unistdx/ipc/identity>

namespace ggg {

    class Transaction;

    using Statement = sqlite::statement;

    class Connection {

    private:
        sqlite::connection_base& _connection;

    public:
        inline explicit Connection(sqlite::connection_base& connection):
        _connection(connection) {}

    public:
        auto find_id_nocheck(const char* name) -> sys::uid_type;
        auto find_id(const char* name) -> sys::uid_type;
        auto find_groups_by_user_name(const char* name) -> Statement;
        auto find_users_by_group_name(const char* name) -> Statement;
        auto children(int64_t id, int depth) -> Statement;
        inline auto children(const char* name, int depth) -> Statement {
            return children(find_id(name), depth);
        }
        auto parents(int64_t id, int depth) -> Statement;
        inline auto parents(const char* name, int depth) -> Statement {
            return parents(find_id(name), depth);
        }

    };

    class Database {

    public:
        enum class File: uint64_t {
            Entities=1,
            Accounts=2,
            All=std::numeric_limits<uint64_t>::max()
        };

        enum class Flag: uint64_t {
            Read_only=1,
            Read_write=2
        };

        using Ties = ggg::Ties;

    public:
        using database_t = sqlite::connection;
        using statement_type = sqlite::statement;
        using group_container_t = std::unordered_map<sys::gid_type,group>;
        using time_point = account::time_point;
        using duration = account::duration;
        using clock_type = account::clock_type;

    private:
        database_t _db;
        time_point _timestamp{duration::zero()};
        char _hostname[HOST_NAME_MAX];
        File _files = File(0);

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

        inline void close() { this->_db.close(); }
        inline database_t* db() { return &this->_db; }
        inline const database_t* db() const { return &this->_db; }
        inline time_point timestamp() const { return this->_timestamp; }

        void optimise();

        statement_type
        find_entity(int64_t id);

        statement_type
        find_entity(const char* name);

        statement_type
        entities();

        statement_type search_entities();
        statement_type search_accounts();
        statement_type search_public_keys();
        statement_type search_messages();
        statement_type search_machines();

        inline statement_type
        find_user(sys::uid_type uid) {
            return this->find_entity(static_cast<int64_t>(uid));
        }

        inline statement_type
        find_user(const char* name) {
            return this->find_entity(name);
        }

        template <class Iterator>
        inline statement_type
        find_non_existing_entities(Iterator first, Iterator last) {
            auto n = std::distance(first, last);
            auto st = this->_db.prepare(
                this->select_non_existing_users_by_names(n).data());
            bind_all(st, first, last);
            return st;
        }

        template <class Iterator>
        inline statement_type
        find_entities(Iterator first, Iterator last) {
            auto n = std::distance(first, last);
            if (n == 0) { return entities(); }
            auto st = this->_db.prepare(this->select_users_by_names(n).data());
            bind_all(st, first, last);
            return st;
        }

        inline statement_type
        users() {
            return this->entities();
        }

        void
        update(const entity& ent);

        bool find_group(sys::gid_type gid, ggg::group& result);
        bool find_group(const char* name, ggg::group& result);
        auto find_groups_by_user_name(const char* name) -> Statement;
        auto find_users_by_group_name(const char* name) -> Statement;

        group_container_t groups();

        statement_type ties();

        sys::uid_type
        next_entity_id();

        void insert(const entity& ent);
        void erase(const entity& ent);

        sys::uid_type
        find_id(const char* name);

        inline bool
        has_entity(const char* name) {
            return this->find_id_nocheck(name) != ggg::bad_uid;
        }

        std::string
        find_name(sys::uid_type id);

        void
        dot(std::ostream& out);

        statement_type
        accounts();

        statement_type
        find_account(const char* name);

        template <class Iterator>
        inline statement_type
        find_accounts(Iterator first, Iterator last) {
            auto n = std::distance(first, last);
            if (n == 0) { return accounts(); }
            auto st = this->_db.prepare(this->select_accounts_by_names(n).data());
            bind_all(st, first, last);
            return st;
        }

        void insert(const account& acc);
        void update(const account& acc, bool update_password=false);
        void erase(const account& acc);

        void set_password(const account& acc);
        void set_last_active(const account& acc, time_point now);

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

        void attach(const char* child, const char* parent, Ties tie);
        void attach(sys::uid_type child_id, sys::gid_type parent_id, Ties tie);
        void detach(sys::uid_type child_id);
        void detach(const char* child);

        void tie(sys::uid_type uid, sys::gid_type gid, Ties tie);
        void tie(const char* child, const char* parent, Ties tie);
        void untie(const char* child, const char* parent);
        void untie(const char* child);

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
        statement_type find_machine(const char* name);
        statement_type machines();
        void insert(const Machine& rhs);
        void erase(const Machine& rhs);
        void remove_all_machines();

        template <class Iterator>
        inline statement_type
        find_machines(Iterator first, Iterator last) {
            auto n = std::distance(first, last);
            if (n == 0) { return machines(); }
            auto st = this->_db.prepare(this->select_machines_by_names(n).data());
            bind_all(st, first, last);
            return st;
        }

        statement_type forms();
        statement_type find_form(const char* name);

        inline void
        prepare_message() {
            if (this->_timestamp != time_point(duration::zero())) return;
            this->_timestamp = clock_type::now();
            ::gethostname(this->_hostname, HOST_NAME_MAX);
        }

        void message(const char* username, time_point t, const char* hostname,
                const char* text);

        inline void
        message(const char* username, const char* text) {
            this->prepare_message();
            this->message(username, this->_timestamp, this->_hostname, text);
        }

        template <class ... Args>
        inline void
        message(const char* username, time_point t, const char* hostname,
                const char* format, const Args& ... args) {
            this->message(username, t, hostname, sqlite::format(format, args...).get());
        }

        template <class ... Args>
        inline void
        message(const char* username, const char* format, const Args& ... args) {
            this->message(username, sqlite::format(format, args...).get());
        }

        statement_type messages();
        statement_type messages(const char* user);

        template <class Iterator>
        inline statement_type
        messages(Iterator first, Iterator last) {
            auto n = std::distance(first, last);
            if (n == 0) { return messages(); }
            auto st = this->_db.prepare(this->select_messages_by_name(n).data());
            int i = 0;
            while (first != last) { st.bind(++i, *first); ++first; }
            return st;
        }

        template <class Iterator>
        inline void
        rotate_messages(Iterator first, Iterator last) {
            auto n = std::distance(first, last);
            auto st = this->_db.prepare(this->rotate_messages(n).data());
            int i = 0;
            while (first != last) { st.bind(++i, *first); ++first; }
            st.step();
            st.close();
        }

        int64_t messages_size();

        statement_type public_keys();
        statement_type public_keys(const char* user);
        void insert(const public_key& rhs);
        void update(const public_key& rhs);
        void erase(const public_key& rhs);

        template <class Iterator>
        inline statement_type
        public_keys(Iterator first, Iterator last) {
            auto n = std::distance(first, last);
            if (n == 0) { return public_keys(); }
            auto st = this->_db.prepare(this->select_public_keys_by_names(n).data());
            bind_all(st, first, last);
            return st;
        }

    private:

        template <class Iterator>
        inline void
        bind_all(statement_type& st, Iterator first, Iterator last) {
            int i = 0;
            while (first != last) { st.bind(++i, *first); ++first; }
        }

        std::string select_non_existing_users_by_names(int n);
        std::string select_users_by_names(int n);
        std::string select_accounts_by_names(int n);
        std::string select_messages_by_name(int n);
        std::string rotate_messages(int n);
        std::string select_machines_by_names(int n);
        std::string select_public_keys_by_names(int n);
        void attach(File file, Flag flag=Flag::Read_only);
        void validate_entity(const entity& ent);
        sys::uid_type find_id_nocheck(const char* name);
        entity find_entity_nocheck(int64_t id);
        entity find_entity_nocheck(const char* name);
        std::string find_name_nocheck(sys::uid_type id);

        friend class Transaction;

    };

    typedef sqlite::row_iterator<ggg::entity> user_iterator;
    typedef sqlite::row_iterator<ggg::group> group_iterator;
    typedef sqlite::row_iterator<ggg::account> account_iterator;
    typedef sqlite::row_iterator<ggg::host> host_iterator;
    typedef sqlite::row_iterator<ggg::Machine> machine_iterator;
    typedef sqlite::row_iterator<ggg::message> message_iterator;

    class Transaction: public sqlite::immediate_transaction {

    public:

        inline explicit
        Transaction(Database& db):
        sqlite::immediate_transaction(db._db) {}

    };

    inline Database::File
    operator|(Database::File a, Database::File b) {
        using tp = std::underlying_type<Database::File>::type;
        return Database::File(tp(a) | tp(b));
    }

    inline std::underlying_type<Database::File>::type
    operator&(Database::File a, Database::File b) {
        using tp = std::underlying_type<Database::File>::type;
        return tp(a) & tp(b);
    }

    inline Database::File
    operator~(Database::File a) {
        using tp = std::underlying_type<Database::File>::type;
        return Database::File(~tp(a));
    }

}

#endif // vim:filetype=cpp
