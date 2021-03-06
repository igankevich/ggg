#include <limits>
#include <regex>
#include <type_traits>
#include <vector>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/schema.hh>

#include <iostream>
#include <iomanip>

namespace {

const char* sql_select_user_by_id = R"(
SELECT id,name,description,home,shell
FROM entities
WHERE id = ?
)";

const char* sql_select_user_by_name = R"(
SELECT id,name,description,home,shell
FROM entities
WHERE name = ?
)";

const char* sql_select_all_users = R"(
SELECT id,name,description,home,shell
FROM entities
)";

const char* sql_search_entities = R"(
SELECT id,name,description,home,shell
FROM entities
WHERE search(name) OR search(description)
)";

const char* sql_update_user_by_id = R"(
UPDATE entities
SET name=?,description=?,home=?,shell=?
WHERE id = ?
)";

const char* sql_insert_user = R"(
INSERT INTO entities
    (id,name,description,home,shell)
VALUES (?,?,?,?,?)
)";

const char* sql_select_next_id = R"(
SELECT COALESCE(MAX(id)+1,$min_id) FROM entities
)";


const char* sql_select_id_by_name = R"(
SELECT id
FROM entities
WHERE name = ?
)";

const char* sql_select_name_by_id = R"(
SELECT name
FROM entities
WHERE id = ?
)";

const char* sql_select_users_by_multiple_names = R"(
SELECT id,name,description,home,shell
FROM entities
WHERE name IN (
)";

const char* sql_select_expired_entities = R"(
SELECT id,entities.name,description,home,shell
FROM entities JOIN accounts ON entities.name=accounts.name
WHERE expiration_date IS NOT NULL
  AND 0 < expiration_date
  AND expiration_date < strftime('%s', 'now')
)";

const char* sql_select_groups_by_user_id = R"(
WITH RECURSIVE
    -- select parent users
    users(id,depth) AS (
        SELECT parent_id,1 FROM ties
        WHERE child_id=$id AND type=0 AND 0<$max
        UNION ALL
        SELECT parent_id,depth+1
        FROM users, ties
        WHERE id = child_id AND ties.type=0 AND depth<$max
    ),
    -- select direct groups
    groups(id,depth) AS (
        SELECT parent_id,1 FROM ties
        WHERE type=1 AND child_id IN (SELECT id FROM users UNION ALL SELECT $id)
        UNION ALL
        SELECT parent_id,depth+1 FROM groups, ties
        WHERE type=1 AND id=child_id AND depth<$max
    ),
    -- select child groups
    child_groups(id,depth) AS (
        SELECT id,depth FROM groups
        UNION ALL
        SELECT child_id,depth FROM child_groups, ties
        WHERE child_groups.id=ties.parent_id AND ties.type=2 AND depth<$max
    )
-- select groups without children
SELECT id,name,description,home,shell
FROM entities
WHERE id IN (SELECT DISTINCT id FROM child_groups
             WHERE id NOT IN (SELECT parent_id FROM ties WHERE type=2))
)";

const char* sql_select_users_by_group_id = R"(
WITH RECURSIVE
    -- select parent groups
    parent_groups(id,depth) AS (
        SELECT parent_id,1 FROM ties
        WHERE child_id = $id AND type=2 AND 0<$max
        UNION ALL
        SELECT parent_id,depth+1 FROM parent_groups, ties
        WHERE id = child_id AND type=2 AND depth<$max
    ),
    -- select direct users
    users(id,depth) AS (
        SELECT child_id,1 FROM ties
        WHERE type=1 AND parent_id IN (SELECT id FROM parent_groups UNION ALL SELECT $id)
        UNION ALL
        SELECT child_id,depth+1 FROM users, ties
        WHERE type=1 AND id=parent_id AND depth<$max
    ),
    -- select child users
    child_users(id,depth) AS (
        SELECT id,1 FROM users
        UNION ALL
        SELECT child_id,depth+1 FROM child_users, ties
        WHERE child_users.id=ties.parent_id AND ties.type=0 AND depth<$max
    )
-- select users without children
SELECT id,name,description,home,shell
FROM entities
WHERE id IN (SELECT DISTINCT id FROM child_users
             WHERE id NOT IN (SELECT parent_id FROM ties WHERE type=0))
)";

const char* sql_select_user_names_by_group_id = R"(
WITH RECURSIVE
    -- select parent groups
    parent_groups(id,depth) AS (
        SELECT parent_id,1 FROM ties
        WHERE child_id = $id AND type=2 AND 0<$max
        UNION ALL
        SELECT parent_id,depth+1 FROM parent_groups, ties
        WHERE id = child_id AND type=2 AND depth<$max
    ),
    -- select direct users
    users(id,depth) AS (
        SELECT child_id,1 FROM ties
        WHERE type=1 AND parent_id IN (SELECT id FROM parent_groups UNION ALL SELECT $id)
        UNION ALL
        SELECT child_id,depth+1 FROM users, ties
        WHERE type=1 AND id=parent_id AND depth<$max
    ),
    -- select child users
    child_users(id,depth) AS (
        SELECT id,1 FROM users
        UNION ALL
        SELECT child_id,depth+1 FROM child_users, ties
        WHERE child_users.id=ties.parent_id AND ties.type=0 AND depth<$max
    )
-- select users without children
SELECT name FROM entities
WHERE id IN (SELECT DISTINCT id FROM child_users
             WHERE id NOT IN (SELECT parent_id FROM ties WHERE type=0))
)";

const char* sql_select_children = R"(
WITH RECURSIVE
    children(id,depth) AS (
        SELECT child_id,1 FROM ties WHERE parent_id=$id AND 0<$max
        UNION ALL
        SELECT child_id,depth+1 FROM children, ties
        WHERE children.id=ties.parent_id AND depth<$max
    )
SELECT id,name,description,home,shell
FROM entities
WHERE id IN (SELECT id FROM children)
)";

const char* sql_select_parents = R"(
WITH RECURSIVE
    parents(id,depth) AS (
        SELECT parent_id,1 FROM ties WHERE child_id=$id AND 0<$max
        UNION ALL
        SELECT parent_id,depth+1 FROM parents, ties
        WHERE parents.id=ties.child_id AND depth<$max
    )
SELECT id,name,description,home,shell
FROM entities
WHERE id IN (SELECT id FROM parents)
)";

const char* sql_delete_tie_by_child_and_parent_name = R"(
DELETE FROM ties
WHERE child_id IN (SELECT id FROM entities WHERE name=$child_name)
  AND parent_id IN (SELECT id FROM entities WHERE name=$parent_name)
)";

const char* sql_select_account_by_name = R"(
SELECT name,password,expiration_date,flags,max_inactive,last_active
FROM accounts
WHERE name = ?
)";

const char* sql_select_accounts_by_multiple_names = R"(
SELECT name,password,expiration_date,flags,max_inactive,last_active
FROM accounts
WHERE name IN (
)";

const char* sql_select_all_accounts = R"(
SELECT name,password,expiration_date,flags,max_inactive,last_active
FROM accounts
)";

const char* sql_insert_account = R"(
INSERT INTO accounts (name,password,expiration_date,flags,max_inactive,last_active)
VALUES (?,?,?,?,?,?)
)";

const char* sql_expire_account_by_name = R"(
UPDATE accounts
SET expiration_date=strftime('%s', 'now')-1
WHERE name = ?
)";

const char* sql_set_password_by_name = R"(
UPDATE accounts
SET password=$password,flags=flags&(~$flag)
WHERE name = $name
)";

const char* sql_set_account_flag_by_name = R"(
UPDATE accounts
SET flags = flags | $flag
WHERE name = $name
)";

const char* sql_unset_account_flag_by_name = R"(
UPDATE accounts
SET flags = flags & (~$flag)
WHERE name = $name
)";

const char* sql_select_entities_by_flag = R"(
SELECT id,entities.name,description,home,shell
FROM entities JOIN accounts ON entities.name = accounts.name
WHERE (flags & $flag) = $value
)";

const char* sql_select_expired_ids = R"(
SELECT id
FROM entities JOIN accounts ON entities.name = accounts.name
WHERE expiration_date IS NOT NULL
  AND 0 < expiration_date
  AND expiration_date < strftime('%s', 'now')
)";

const char* sql_select_expired_names = R"(
SELECT name
FROM entities
WHERE expiration_date IS NOT NULL
  AND 0 < expiration_date
  AND expiration_date < strftime('%s', 'now')
)";

const char* sql_select_hierarchy_root_by_id = R"(
WITH RECURSIVE
    parent_entities(id,depth) AS (
        SELECT parent_id,1 FROM ties
        WHERE child_id=$id AND type<>1 AND 0<$max
        UNION ALL
        SELECT parent_id,depth+1 FROM parent_entities, ties
        WHERE id=child_id AND type<>1 AND depth<$max
    )
SELECT id FROM parent_entities WHERE depth IN (SELECT MAX(depth) FROM parent_entities)
)";

const char* sql_select_all_hosts = R"(
SELECT ethernet_address,name
FROM hosts
)";

const char* sql_select_host_by_name = R"(
SELECT ethernet_address,name
FROM hosts
WHERE name=?
)";

const char* sql_select_host_by_address = R"(
SELECT ethernet_address,name
FROM hosts
WHERE ethernet_address=?
)";

const char* sql_select_all_host_addresses = R"(
SELECT ip_address,name
FROM hosts JOIN addresses ON hosts.ethernet_address=addresses.ethernet_address
)";

const char* sql_select_ip_address_by_name_and_length = R"(
SELECT ip_address
FROM hosts JOIN addresses ON hosts.ethernet_address=addresses.ethernet_address
WHERE hosts.name=? AND length(hosts.ethernet_address)=?
)";

const char* sql_select_host_name_by_address = R"(
SELECT name
FROM hosts JOIN addresses ON hosts.ethernet_address=addresses.ethernet_address
WHERE ip_address=?
)";

const char* sql_select_all_machines = R"(
SELECT name, addresses.ethernet_address, addresses.ip_address
FROM hosts
JOIN addresses ON hosts.ethernet_address=addresses.ethernet_address
)";

const char* sql_select_machine_by_name = R"(
SELECT name, addresses.ethernet_address, addresses.ip_address
FROM hosts
JOIN addresses ON hosts.ethernet_address=addresses.ethernet_address
WHERE name=?
)";

const char* sql_select_machines_by_multiple_names = R"(
SELECT name, addresses.ethernet_address, addresses.ip_address
FROM hosts
JOIN addresses ON hosts.ethernet_address=addresses.ethernet_address
WHERE name IN (
)";

    struct Tie {

        sys::uid_type child_id = ggg::bad_uid;
        sys::uid_type parent_id = ggg::bad_uid;
        ggg::Database::Ties type{};

        Tie() = default;

        Tie(sys::uid_type id1, sys::uid_type id2):
        child_id(id1), parent_id(id2) {}

        void
        dot(std::ostream& out) const {
            using t = ggg::Database::Ties;
            switch (type) {
                case t::User_user:
                    out << "id" << child_id << " -> " << "id" << parent_id;
                    out << "; // uu\n";
                    break;
                case t::User_group:
                    out << "id" << child_id << " -> " << "id" << parent_id;
                    out << " [constraint=\"false\"]";
                    out << "; // ug\n";
                    break;
                case t::Group_group:
                    out << "id" << parent_id << " -> " << "id" << child_id;
                    out << "; // gg\n";
                    break;
            }
        }

    };

    void
    operator>>(const sqlite::statement& in, Tie& rhs) {
        sqlite::cstream cstr(in);
        int type = 0;
        cstr >> rhs.child_id >> rhs.parent_id >> type;
        rhs.type = ggg::Database::Ties(type);
    }

    struct database_parameters {
        const char* filename;
        const char* const* schema;
        const int64_t schema_version;
        const char* name;
    };

    const database_parameters configurations[] = {
        {
            GGG_ENTITIES_PATH,
            entities_schema,
            entities_schema_version,
            "entities"
        },
        {
            GGG_ACCOUNTS_PATH,
            accounts_schema,
            accounts_schema_version,
            "accounts"
        },
    };

}

void
ggg::Database::open(File file, Flag flag) {
    this->close();
    this->_files = file;
    // open entities implicitly
    // if (file & ~File::Entities) { file = file | File::Entities; }
    // convert flags to sqlite
    sqlite::file_flag flags = sqlite::file_flag::read_only;
    if (flag == Flag::Read_write) {
        flags = sqlite::file_flag::read_write | sqlite::file_flag::create;
    }
    int nfiles = 2;
    int nopen = 0;
    for (int i=0; i<nfiles; ++i) {
        if (!(file & File(1<<i))) { continue; }
        const auto& params = configurations[i];
        if (nopen == 0) {
            this->_db.open(params.filename, flags);
            ++nopen;
            int64_t version = this->_db.user_version();
            if (version < params.schema_version) {
                if (flag == Flag::Read_only) {
                    throw std::invalid_argument(
                        "unable to update schema of read-only database"
                    );
                }
                for (int64_t v=version; v<params.schema_version; ++v) {
                    this->_db.execute(params.schema[v]);
                    this->_db.user_version(v+1);
                }
            }
        } else {
            this->_db.attach(params.filename, params.name);
        }
    }
    this->_db.foreign_keys(true);
    this->_db.busy_timeout(std::chrono::seconds(30));
}

void
ggg::Database::attach(File file, Flag flag) {
    int nfiles = 2;
    for (int i=0; i<nfiles; ++i) {
        if (!(file & File(1<<i))) { continue; }
        const auto& params = configurations[i];
        this->_db.attach(params.filename, params.name);
        this->_files = this->_files | file;
    }
}

auto
ggg::Database::find_entity(int64_t id) -> statement_type {
    return this->_db.prepare(sql_select_user_by_id, id);
}

auto
ggg::Database::find_entity(const char* name) -> statement_type {
    return this->_db.prepare(sql_select_user_by_name, name);
}

auto
ggg::Database::entities() -> statement_type {
    return this->_db.prepare(sql_select_all_users);
}

auto
ggg::Database::search_entities() -> statement_type {
    return this->_db.prepare(sql_search_entities);
}

auto
ggg::Database::search_accounts() -> statement_type {
    return this->_db.prepare(R"(
SELECT name,password,expiration_date,flags,max_inactive,last_active
FROM accounts
WHERE search(name)
)");
}

auto
ggg::Database::search_public_keys() -> statement_type {
    return this->_db.prepare("SELECT account_name,options,type,key,comment "
            "FROM public_keys WHERE search(key) OR search(comment)");
}

auto
ggg::Database::search_messages() -> statement_type {
    return this->_db.prepare("SELECT account_name,timestamp,machine_name,message "
            "FROM messages WHERE search(message) ORDER BY timestamp");
}

auto
ggg::Database::search_machines() -> statement_type {
    return this->_db.prepare("SELECT name, addresses.ethernet_address, "
            "addresses.ip_address FROM hosts "
            "JOIN addresses ON hosts.ethernet_address=addresses.ethernet_address "
            "WHERE search(name)");
}

bool
ggg::Database::find_group(sys::gid_type gid, ggg::group& result) {
    auto st = this->_db.prepare("SELECT id,name,description FROM entities WHERE id=?", gid);
    if (st.step() == sqlite::errc::done) { return false; }
    st >> result;
    st.close();
    st = this->_db.prepare(sql_select_user_names_by_group_id, gid, GGG_MAX_DEPTH);
    ggg::group::container_type members;
    std::string name;
    while (st.step() != sqlite::errc::done) {
        st.column(0, name);
        members.emplace(std::move(name));
    }
    result.members(std::move(members));
    return true;
}

bool
ggg::Database::find_group(const char* name, ggg::group& result) {
    auto id = find_id(name);
    return find_group(id, result);
}

auto ggg::Database::find_groups_by_user_name(const char* name) -> Statement {
    return Connection(this->_db).find_groups_by_user_name(name);
}

auto ggg::Database::find_users_by_group_name(const char* name) -> Statement {
    return Connection(this->_db).find_users_by_group_name(name);
}

sys::uid_type ggg::Connection::find_id_nocheck(const char* name) {
    sys::uid_type id = bad_uid;
    auto st = this->_connection.prepare(sql_select_id_by_name, name);
    if (st.step() != sqlite::errc::done) { st.column(0, id); }
    return id;
}

sys::uid_type ggg::Connection::find_id(const char* name) {
    sys::uid_type id = find_id_nocheck(name);
    if (id == bad_uid) { throw std::invalid_argument("bad name"); }
    return id;
}

auto ggg::Connection::find_groups_by_user_name(const char* name) -> Statement {
    auto id = find_id(name);
    return this->_connection.prepare(sql_select_groups_by_user_id, id, GGG_MAX_DEPTH);
}

auto ggg::Connection::find_users_by_group_name(const char* name) -> Statement {
    auto id = find_id(name);
    return this->_connection.prepare(sql_select_users_by_group_id, id, GGG_MAX_DEPTH);
}

auto ggg::Connection::children(int64_t id, int depth) -> Statement {
    return this->_connection.prepare(sql_select_children, id, depth);
}

auto ggg::Connection::parents(int64_t id, int depth) -> Statement {
    return this->_connection.prepare(sql_select_parents, id, depth);
}

auto
ggg::Database::groups() -> group_container_t {
    group_container_t groups;
    {
        auto st = this->_db.prepare("SELECT id,name,description FROM entities");
        for (auto& tmp : st.rows<group>()) {
            groups.emplace(tmp.id(), std::move(tmp));
        }
    }
    for (auto& pair : groups) {
        auto st = this->_db.prepare(sql_select_user_names_by_group_id,
                                    pair.first, GGG_MAX_DEPTH);
        std::string name;
        while (st.step() != sqlite::errc::done) {
            st.column(0, name);
            pair.second.push(std::move(name));
        }
    }
    return groups;
}

sys::uid_type
ggg::Database::next_entity_id() {
    sys::uid_type id = bad_uid;
    auto rstr = this->_db.prepare(sql_select_next_id, GGG_MIN_ID);
    if (rstr.step() != sqlite::errc::done) {
        rstr.column(0, id);
    }
    if (id == bad_uid) {
        throw std::invalid_argument("failed to generate new id");
    }
    for (int i=0; i<2; ++i) {
        if (id == std::numeric_limits<sys::uid_type>::max()) {
            throw std::overflow_error("id overflow");
        }
        if (id == GGG_OVERFLOW_ID) {
            ++id;
        }
        if (id < GGG_MIN_ID) {
            id = GGG_MIN_ID;
        }
    }
    return id;
}

void
ggg::Database::validate_entity(const entity& ent) {
    if (!ent.has_valid_name()) {
        throw std::invalid_argument("bad name");
    }
    if (struct ::passwd* pw = ::getpwnam(ent.name().data())) {
        if (pw->pw_uid < GGG_MIN_ID || pw->pw_gid < GGG_MIN_ID) {
            throw std::invalid_argument("conflicting system user");
        }
    }
}

void
ggg::Database::insert(const entity& ent) {
    validate_entity(ent);
    auto home = ent.home().empty() ? nullptr : ent.home().data();
    auto shell = ent.shell().empty() ? nullptr : ent.shell().data();
    sys::uid_type id;
    if (ent.has_id()) {
        id = ent.id();
        if (id < GGG_MIN_ID || id == GGG_OVERFLOW_ID) {
            throw std::invalid_argument("bad id");
        }
    } else {
        id = this->next_entity_id();
    }
    this->_db.execute(
        sql_insert_user,
        id,
        ent.name(),
        ent.description(),
        home,
        shell
    );
    message(ent.name().data(), "entity created");
    if (ent.has_valid_parent()) {
        auto parent_id = find_id(ent.parent().data());
        attach(id, parent_id, Ties::User_user);
    }
}

void
ggg::Database::erase(const entity& ent) {
    this->_db.execute("DELETE FROM entities WHERE name = ?", ent.name());
    auto nrows1 = this->_db.num_rows_modified();
    this->_db.execute("DELETE FROM accounts WHERE name=?", ent.name());
    auto nrows2 = this->_db.num_rows_modified();
    if (nrows1 == 0 && nrows2 == 0) {
        throw std::invalid_argument("bad entity");
    }
    message(ent.name().data(), "entity removed");
    message(ent.name().data(), "account removed");
}

void
ggg::Database::erase(const account& acc) {
    this->_db.execute("DELETE FROM accounts WHERE name=?", acc.name());
    auto nrows = this->_db.num_rows_modified();
    if (nrows == 0) { throw std::invalid_argument("bad account"); }
    message(acc.name().data(), "account removed");
}

ggg::entity
ggg::Database::find_entity_nocheck(const char* name) {
    entity ent;
    auto st = this->find_entity(name);
    if (st.step() != sqlite::errc::done) { st >> ent; }
    return ent;
}

ggg::entity
ggg::Database::find_entity_nocheck(int64_t id) {
    entity ent;
    auto st = this->find_entity(id);
    if (st.step() != sqlite::errc::done) { st >> ent; }
    return ent;
}

sys::uid_type
ggg::Database::find_id_nocheck(const char* name) {
    sys::uid_type id = bad_uid;
    auto rstr = this->_db.prepare(sql_select_id_by_name, name);
    if (rstr.step() != sqlite::errc::done) { rstr.column(0, id); }
    return id;
}

sys::uid_type
ggg::Database::find_id(const char* name) {
    sys::uid_type id = find_id_nocheck(name);
    if (id == bad_uid) { throw std::invalid_argument("bad name"); }
    return id;
}

std::string
ggg::Database::find_name_nocheck(sys::uid_type id) {
    std::string name;
    auto rstr = this->_db.prepare(sql_select_name_by_id, id);
    if (rstr.step() != sqlite::errc::done) { rstr.column(0, name); }
    return name;
}

std::string
ggg::Database::find_name(sys::uid_type id) {
    std::string name = find_name_nocheck(id);
    if (name.empty()) {
        throw std::invalid_argument("bad id");
    }
    return name;
}

auto
ggg::Database::ties() -> statement_type {
    return this->_db.prepare("SELECT child_id,parent_id,type FROM ties");
}

void
ggg::Database::dot(std::ostream& out) {
    sqlite::deferred_transaction tr(this->_db);
    sqlite::statement st;
    out << "digraph GGG {\n";
    out << "rankdir=LR;\n";
    st = entities();
    for (const auto& ent : st.rows<entity>()) {
        out << "id" << ent.id() << " [label=\"" << ent.name() << "\"];\n";
    }
    st = ties();
    std::unordered_map<sys::uid_type,std::vector<Tie>> hierarchies;
    for (const auto& tie : st.rows<Tie>()) {
        if (tie.type != Ties::User_group) {
            auto root_id = find_hierarchy_root(tie.child_id);
            hierarchies[root_id].emplace_back(tie);
        } else {
            tie.dot(out);
        }
    }
    for (const auto& entry : hierarchies) {
        auto root_id = entry.first;
        auto root_name = find_name(root_id);
        const auto& ties = entry.second;
        out << "subgraph cluster_" << root_id << " {\n";
        out << "label=\"" << root_name << "\";\n";
        for (const auto& tie : ties) { tie.dot(out); }
        out << "}\n";
    }
    st.close();
    out << "}\n";
    tr.commit();
}

auto
ggg::Database::find_account(const char* name) -> statement_type {
    return this->_db.prepare(sql_select_account_by_name, name);
}

auto
ggg::Database::accounts() -> statement_type {
    return this->_db.prepare(sql_select_all_accounts);
}

void
ggg::Database::insert(const account& acc) {
    using int_type = std::underlying_type<account_flags>::type;
    if (!has_entity(acc.name().data())) {
        throw std::invalid_argument("bad account");
    }
    this->_db.execute(
        sql_insert_account,
        acc.name(),
        acc.password().empty() ? nullptr : acc.password().data(),
        acc.expire(),
        static_cast<int_type>(acc.flags()),
        acc.max_inactive_in_seconds(),
        acc.last_active()
    );
    message(acc.name().data(), "account created");
}

void
ggg::Database::update(const account& acc, bool update_password) {
    using int_type = std::underlying_type<account_flags>::type;
    if (update_password) {
        this->_db.execute("UPDATE accounts "
                          "SET password=?,expiration_date=?,flags=?,max_inactive=?,last_active=? "
                          "WHERE name = ?",
            acc.password().empty() ? nullptr : acc.password().data(),
            acc.expire(),
            static_cast<int_type>(acc.flags()),
            acc.max_inactive_in_seconds(),
            acc.last_active(),
            acc.name());
    } else {
        this->_db.execute("UPDATE accounts "
                          "SET expiration_date=?,flags=?,max_inactive=?,last_active=? "
                          "WHERE name = ?",
            acc.expire(),
            static_cast<int_type>(acc.flags()),
            acc.max_inactive_in_seconds(),
            acc.last_active(),
            acc.name());
    }
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad account");
    }
    message(acc.name().data(), "account updated");
}

void
ggg::Database::set_password(const account& acc) {
    using int_type = std::underlying_type<account_flags>::type;
    this->_db.execute(
        sql_set_password_by_name,
        acc.password().data(),
        static_cast<int_type>(account_flags::password_has_expired),
        acc.name()
    );
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad account");
    }
    message(acc.name().data(), "password changed");
}

void
ggg::Database::set_last_active(const account& acc, time_point now) {
    auto was_active = !acc.is_inactive(now);
    this->_db.execute(
        "UPDATE accounts SET last_active=? WHERE name=?",
        acc.last_active(),
        acc.name()
    );
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad account");
    }
    auto now_active = !acc.is_inactive(now);
    if (was_active && !now_active) {
        message(acc.name().data(), "account is deactivated");
    } else if (!was_active && now_active) {
        message(acc.name().data(), "account is activated");
    }
}

void
ggg::Database::expire(const char* name) {
    this->_db.execute(sql_expire_account_by_name, name);
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad account name");
    }
    message(name, "account expired");
}

void
ggg::Database::set_account_flag(const char* name, account_flags flag) {
    using int_type = std::underlying_type<account_flags>::type;
    this->_db.execute(sql_set_account_flag_by_name, static_cast<int_type>(flag), name);
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad account name");
    }
    if (flag & account_flags::suspended) {
        message(name, "account locked");
    }
}

void
ggg::Database::unset_account_flag(const char* name, account_flags flag) {
    typedef std::underlying_type<account_flags>::type int_type;
    this->_db.execute(
        sql_unset_account_flag_by_name,
        static_cast<int_type>(flag),
        name
    );
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad account name");
    }
    if (flag & account_flags::suspended) {
        message(name, "account unlocked");
    }
}

std::string
ggg::Database::select_non_existing_users_by_names(int n) {
    if (n == 0) { return "SELECT name FROM entities WHERE FALSE"; }
    std::string sql;
    sql.reserve(4096);
    sql += "WITH needed(name) AS (";
    for (int i=0; i<n; ++i) {
        sql += "\nVALUES (?) UNION ALL";
    }
    for (int i=0; i<9; ++i) { sql.pop_back(); }
    sql.back() = ')';
    sql += "\nSELECT name FROM needed WHERE name NOT IN (SELECT name FROM entities)";
    return sql;
}

std::string
ggg::Database::select_users_by_names(int n) {
    std::string sql(sql_select_users_by_multiple_names);
    for (int i=0; i<n; ++i) {
        sql += '?';
        sql += ',';
    }
    sql.back() = ')';
    return sql;
}

std::string
ggg::Database::select_machines_by_names(int n) {
    std::string sql(sql_select_machines_by_multiple_names);
    for (int i=0; i<n; ++i) {
        sql += '?';
        sql += ',';
    }
    sql.back() = ')';
    return sql;
}



std::string
ggg::Database::select_accounts_by_names(int n) {
    std::string sql(sql_select_accounts_by_multiple_names);
    for (int i=0; i<n; ++i) {
        sql += '?';
        sql += ',';
    }
    sql.back() = ')';
    return sql;
}

void
ggg::Database::update(const entity& ent) {
    if (!ent.has_id() && !ent.has_name()) {
        throw std::invalid_argument("bad entity");
    }
    validate_entity(ent);
    auto id = ent.id();
    auto name = ent.name();
    entity old_ent;
    if (ent.has_id() && ent.has_name()) {
        auto existing_id = find_id_nocheck(ent.name().data());
        auto existing_name = find_name_nocheck(ent.id());
        if (existing_id != bad_uid &&
            existing_id != ent.id() &&
            existing_name == ent.name()) {
            throw std::invalid_argument("changing id is not allowed");
        }
        old_ent = find_entity_nocheck(ent.id());
    } else if (!ent.has_id()) {
        old_ent = find_entity_nocheck(ent.name().data());
        id = old_ent.id();
    } else if (!ent.has_name()) {
        old_ent = find_entity_nocheck(ent.id());
        name = old_ent.name();
    }
    this->_db.execute(sql_update_user_by_id, name, ent.description(),
            ent.home(), ent.shell(), id);
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad entity");
    }
    if (name != old_ent.name()) {
        auto old_name = old_ent.name().data();
        message(old_name, "entity name changed to %s", name.data());
        message(name.data(), "entity renamed from %s", old_name);
    }
    if (ent.description() != old_ent.description()) {
        message(name.data(), "entity real name changed to %s", ent.description().data());
    }
    if (ent.home() != old_ent.home()) {
        message(name.data(), "entity home directory changed to %s", ent.home().data());
    }
    if (ent.shell() != old_ent.shell()) {
        message(name.data(), "entity shell changed to %s", ent.shell().data());
    }
}

auto
ggg::Database::expired_entities() -> statement_type {
    return this->_db.prepare(sql_select_expired_entities);
}

auto
ggg::Database::expired_ids() -> statement_type {
    return this->_db.prepare(sql_select_expired_ids);
}

auto
ggg::Database::expired_names() -> statement_type {
    return this->_db.prepare(sql_select_expired_names);
}

auto
ggg::Database::find_entities_by_flag(
    account_flags flag,
    bool set
) -> statement_type {
    using int_type = std::underlying_type<account_flags>::type;
    int_type v = static_cast<int_type>(flag);
    return this->_db.prepare(sql_select_entities_by_flag, v, set ? v : 0);
}

sys::uid_type
ggg::Database::find_hierarchy_root(sys::uid_type id) {
    sys::uid_type root_id = bad_uid;
    auto st = this->_db.prepare(sql_select_hierarchy_root_by_id, id, GGG_MAX_DEPTH);
    if (st.step() != sqlite::errc::done) { st.column(0, root_id); }
    if (root_id == bad_uid) { root_id = id; }
    return root_id;
}

void
ggg::Database::detach(sys::uid_type id) {
    this->_db.execute("DELETE FROM ties WHERE child_id=$id", id);
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad id");
    }
}

void
ggg::Database::detach(const char* name) {
    this->_db.execute(
        "DELETE FROM ties WHERE child_id IN (SELECT id FROM entities WHERE name=$name)",
        name);
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad name");
    }
}

void
ggg::Database::attach(sys::uid_type child_id, sys::gid_type parent_id, Ties tie) {
    if (child_id == parent_id) { throw std::invalid_argument("self-loop"); }
    if (entities_are_tied(child_id, parent_id)) {
        throw std::invalid_argument("entities are tied");
    }
    this->_db.execute("INSERT INTO ties(child_id,parent_id,type) VALUES (?,?,?)",
                      child_id, parent_id, int(tie));
}

void
ggg::Database::attach(const char* child, const char* parent, Ties tie) {
    auto child_id = find_id(child);
    auto parent_id = find_id(parent);
    attach(child_id, parent_id, tie);
}

void
ggg::Database::tie(sys::uid_type uid, sys::gid_type gid, Ties tie) {
    if (uid == gid) { throw std::invalid_argument("self-loop"); }
    if (entities_are_tied(uid, gid)) {
        this->_db.execute("UPDATE ties SET type=? WHERE child_id=? AND parent_id=?",
                          int(tie), uid, gid);
    } else {
        this->_db.execute("INSERT INTO ties(child_id,parent_id,type) VALUES (?,?,?)",
                          uid, gid, int(tie));
    }
}

void
ggg::Database::tie(const char* child, const char* parent, Ties tie) {
    this->tie(find_id(child), find_id(parent), tie);
}

void
ggg::Database::untie(const char* child, const char* parent) {
    this->_db.execute(sql_delete_tie_by_child_and_parent_name, child, parent);
    if (this->_db.num_rows_modified() == 0) {
        throw std::invalid_argument("bad names");
    }
}

void
ggg::Database::untie(const char* child) {
    auto child_id = find_id(child);
    this->_db.execute("DELETE FROM ties WHERE child_id=$id OR parent_id=$id", child_id);
}

bool
ggg::Database::entities_are_tied(sys::uid_type child_id, sys::gid_type parent_id) {
    auto rstr = this->_db.prepare("SELECT child_id,parent_id FROM ties "
                                  "WHERE child_id=$child_id AND parent_id=$parent_id",
                                  child_id, parent_id);
    return rstr.step() != sqlite::errc::done;
}

auto
ggg::Database::hosts() -> statement_type {
    return this->_db.prepare(sql_select_all_hosts);
}

auto
ggg::Database::find_host(const char* name) -> statement_type {
    return this->_db.prepare(sql_select_host_by_name, name);
}

auto
ggg::Database::find_machine(const char* name) -> statement_type {
    return this->_db.prepare(sql_select_machine_by_name, name);
}

auto
ggg::Database::find_host(const sys::ethernet_address& address) -> statement_type {
    return this->_db.prepare(
        sql_select_host_by_address,
        sqlite::blob(address.begin(), address.end())
    );
}

auto
ggg::Database::host_addresses() -> statement_type {
    return this->_db.prepare(sql_select_all_host_addresses);
}

auto
ggg::Database::find_ip_address(
    const char* name,
    sys::family_type family
) -> statement_type {
    int64_t length = family == sys::family_type::inet6
        ? sizeof(sys::ipv6_address)
        : sizeof(sys::ipv4_address);
    return this->_db.prepare(sql_select_ip_address_by_name_and_length, name, length);
}

auto
ggg::Database::find_ip_address(const sys::ethernet_address& hwaddr) -> statement_type {
    return this->_db.prepare("SELECT ip_address FROM addresses WHERE ethernet_address=?",
        sqlite::blob(hwaddr.begin(), hwaddr.end()));
}

auto
ggg::Database::find_host_name(const ip_address& address) -> statement_type {
    return this->_db.prepare(
        sql_select_host_name_by_address,
        sqlite::blob(address.data(), address.size())
    );
}

auto
ggg::Database::machines() -> statement_type {
    return this->_db.prepare(sql_select_all_machines);
}

void
ggg::Database::insert(const Machine& rhs) {
    std::bitset<3> set;
    set[0] = !rhs.name().empty();
    set[1] = rhs.address().family() != sys::family_type::unspecified;
    set[2] = (rhs.ethernet_address() != sys::ethernet_address());
    if (set.count() < 2) { throw std::invalid_argument("bad machine"); }
    const auto& addr = rhs.address();
    auto hwaddr = rhs.ethernet_address();
    if (set[0]) {
        bool insert_hwaddr = false;
        if (set[2]) {
            if (find_host(hwaddr).step() == sqlite::errc::done) {
                insert_hwaddr = true;
            }
        } else {
            auto st = find_host(rhs.name().data());
            if (st.step() == sqlite::errc::done) {
                insert_hwaddr = true;
            } else {
                sqlite::cstream in(st);
                in >> hwaddr;
            }
        }
        if (insert_hwaddr) {
            this->_db.execute("INSERT INTO hosts (ethernet_address,name) VALUES (?,?)",
                sqlite::blob(hwaddr.begin(), hwaddr.end()), rhs.name());
        }
    }
    auto st = this->_db.prepare("SELECT ip_address FROM addresses WHERE ip_address=?",
        sqlite::blob(addr.data(), addr.size()));
    if (st.step() == sqlite::errc::done) {
        this->_db.execute("INSERT INTO addresses (ip_address,ethernet_address) VALUES (?,?)",
            sqlite::blob(addr.data(), addr.size()),
            sqlite::blob(hwaddr.begin(), hwaddr.end()));
    }
}

void
ggg::Database::erase(const Machine& m) {
    this->_db.execute("DELETE FROM hosts WHERE name=?", m.name());
    this->_db.execute("DELETE FROM addresses WHERE ip_address=?",
        sqlite::blob(m.address().data(), m.address().size()));
    this->_db.execute("DELETE FROM hosts WHERE ethernet_address=?",
        sqlite::blob(m.ethernet_address().begin(), m.ethernet_address().end()));
}

void
ggg::Database::remove_all_machines() {
    this->_db.execute("DELETE FROM hosts");
}

auto
ggg::Database::forms() -> statement_type {
    return this->_db.prepare("SELECT name,content FROM forms");
}

auto
ggg::Database::find_form(const char* name) -> statement_type {
    return this->_db.prepare("SELECT name,content FROM forms WHERE name=?", name);
}

void
ggg::Database::message(const char* username, time_point t, const char* hostname,
        const char* text) {
    if (!(this->_files & File::Accounts)) {
        attach(File::Accounts, Flag::Read_write);
    }
    auto st = this->_db.prepare("INSERT INTO messages "
            "(account_name,timestamp,machine_name,message) "
            "VALUES (?,?,?,?)", username, t, hostname, text);
    st.step();
}

auto
ggg::Database::messages() -> statement_type {
    return this->_db.prepare("SELECT account_name,timestamp,machine_name,message "
            "FROM messages ORDER BY timestamp");
}

auto
ggg::Database::messages(const char* user) -> statement_type {
    return this->_db.prepare("SELECT account_name,timestamp,machine_name,message "
            "FROM messages WHERE account_name=? ORDER BY timestamp", user);
}

std::string
ggg::Database::select_messages_by_name(int n) {
    std::string sql;
    sql.reserve(4096);
    sql += "SELECT account_name,timestamp,machine_name,message "
        "FROM messages WHERE account_name IN (";
    for (int i=0; i<n; ++i) {
        sql += '?';
        sql += ',';
    }
    sql.back() = ')';
    sql += " ORDER BY timestamp";
    return sql;
}

std::string
ggg::Database::rotate_messages(int n) {
    std::string sql;
    sql.reserve(4096);
    sql += "DELETE FROM messages WHERE timestamp < strftime('%s', 'now'";
    for (int i=0; i<n; ++i) {
        sql += ',';
        sql += '?';
    }
    sql += ')';
    return sql;
}

int64_t
ggg::Database::messages_size() {
    int64_t size = -1;
    auto st = this->_db.prepare("SELECT SUM(pgsize) FROM dbstat WHERE name='messages'");
    if (st.step() == sqlite::errc::done) { throw std::runtime_error("bad size"); }
    st.column(0, size);
    return size;
}

void
ggg::Database::optimise() {
    this->_db.vacuum();
    this->_db.optimize();
}

void
ggg::Database::insert(const public_key& rhs) {
    this->_db.execute("INSERT INTO public_keys "
            "(account_name,options,type,key,comment) "
            "VALUES (?,?,?,?,?)", rhs.name(),
            rhs.options(), rhs.type(), rhs.key(), rhs.comment());
    message("public key %s added", rhs.key().data());
}

void
ggg::Database::update(const public_key& rhs) {
    this->_db.execute("UPDATE public_keys "
            "SET options=?,type=?,key=?,comment=? "
            "WHERE account_name=?", rhs.options(),
            rhs.type(), rhs.key(), rhs.comment(),
            rhs.name());
}

void
ggg::Database::erase(const public_key& rhs) {
    if (rhs.has_name() && rhs.has_key()) {
        this->_db.execute("DELETE FROM public_keys WHERE account_name=? "
                "AND key=?", rhs.name(), rhs.key());
    } else if (rhs.has_name()) {
        this->_db.execute("DELETE FROM public_keys WHERE account_name=?", rhs.name());
    } else {
        throw std::invalid_argument("bad public key");
    }
    message("public key %s removed", rhs.key().data());
}

auto
ggg::Database::public_keys(const char* user) -> statement_type {
    return this->_db.prepare("SELECT account_name,options,type,key,comment "
            "FROM public_keys WHERE account_name=?", user);
}

auto
ggg::Database::public_keys() -> statement_type {
    return this->_db.prepare("SELECT account_name,options,type,key,comment "
            "FROM public_keys");
}

std::string
ggg::Database::select_public_keys_by_names(int n) {
    std::string sql;
    sql.reserve(4096);
    sql += "SELECT account_name,options,type,key,comment "
        "FROM public_keys WHERE account_name IN (";
    for (int i=0; i<n; ++i) {
        sql += '?';
        sql += ',';
    }
    sql.back() = ')';
    return sql;
}
