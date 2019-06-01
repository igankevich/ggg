#include <limits>
#include <regex>
#include <type_traits>
#include <vector>

#include <ggg/bits/to_bytes.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/schema.hh>

#include <iostream>

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

const char* sql_delete_entity_by_name = R"(
DELETE FROM entities WHERE name = ?
)";

const char* sql_select_group_by_id = R"(
WITH RECURSIVE
	-- merge hierarchy and ties
	edges(child_id,parent_id) AS (
		SELECT child_id,parent_id FROM hierarchy
		UNION
		SELECT child_id,parent_id FROM ties
	),
	-- find all upstream entities in the graph
	path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM edges
		WHERE parent_id = $id
		UNION ALL
		SELECT edges.child_id, edges.parent_id, path.depth+1
		FROM path, edges
		WHERE path.child_id = edges.parent_id
		  AND path.child_id <> path.parent_id
		  AND path.depth < $depth
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT DISTINCT child_id FROM path UNION SELECT $id)
)";

const char* sql_select_group_by_name = R"(
WITH RECURSIVE
	-- find group id by group name
	ids(id) AS (
		SELECT id FROM entities WHERE name = $name
	),
	-- merge hierarchy and ties
	edges(child_id,parent_id) AS (
		SELECT child_id,parent_id FROM hierarchy
		UNION
		SELECT child_id,parent_id FROM ties
	),
	-- find all upstream entities in the graph
	path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM edges
		WHERE parent_id IN (SELECT id FROM ids)
		UNION ALL
		SELECT edges.child_id, edges.parent_id, path.depth+1
		FROM path, edges
		WHERE path.child_id = edges.parent_id
		  AND path.child_id <> path.parent_id
		  AND path.depth < $depth
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT DISTINCT child_id FROM path UNION SELECT id FROM ids)
)";

const char* sql_select_parent_entities_by_id = R"(
WITH RECURSIVE
	-- merge hierarchy and ties
	edges(child_id,parent_id) AS (
		SELECT child_id,parent_id FROM hierarchy
		UNION
		SELECT child_id,parent_id FROM ties
	),
	-- find all downstream entities in the graph
	path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM edges
		WHERE child_id = $id
		UNION ALL
		SELECT edges.child_id, edges.parent_id, path.depth+1
		FROM path, edges
		WHERE path.parent_id = edges.child_id
		  AND path.child_id <> path.parent_id
		  AND path.depth < $depth
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT DISTINCT parent_id FROM path)
)";

const char* sql_select_child_entities_by_id = R"(
WITH RECURSIVE
	-- merge hierarchy and ties
	edges(child_id,parent_id) AS (
		SELECT child_id,parent_id FROM hierarchy
		UNION
		SELECT child_id,parent_id FROM ties
	),
	-- find all upstream entities in the graph
	path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM edges
		WHERE parent_id = $id
		UNION ALL
		SELECT edges.child_id, edges.parent_id, path.depth+1
		FROM path, edges
		WHERE path.child_id = edges.parent_id
		  AND path.child_id <> path.parent_id
		  AND path.depth < $depth
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT DISTINCT child_id FROM path)
)";

const char* sql_select_all_group_members = R"(
WITH RECURSIVE
	-- merge hierarchy and ties
	edges(child_id,parent_id) AS (
		SELECT child_id,parent_id FROM hierarchy
		UNION
		SELECT child_id,parent_id FROM ties
	),
	-- find all upstream entities in the graph
	path(child_id,parent_id,depth,initial_id) AS (
		SELECT child_id,parent_id,1,parent_id FROM edges
		UNION ALL
		SELECT edges.child_id, edges.parent_id, path.depth+1, path.initial_id
		FROM path, edges
		WHERE path.child_id = edges.parent_id
		  AND path.child_id <> path.parent_id
		  AND path.depth < $depth
	)
SELECT child_id,initial_id FROM path
)";

const char* sql_select_all_groups = R"(
SELECT id,name,description
FROM entities
)";

const char* sql_select_all_ties = R"(
SELECT child_id,parent_id FROM ties
)";

const char* sql_select_tie_by_ids = R"(
SELECT child_id,parent_id
FROM ties
WHERE child_id=$child_id AND parent_id=$parent_id
)";

const char* sql_insert_tie = R"(
INSERT INTO ties (child_id,parent_id) VALUES (?,?)
)";

const char* sql_delete_tie_by_id = R"(
DELETE FROM ties
WHERE child_id=$id OR parent_id=$id
)";

const char* sql_delete_tie_by_child_and_parent_name = R"(
DELETE FROM ties
WHERE child_id IN (SELECT id FROM entities WHERE name=$child_name)
  AND parent_id IN (SELECT id FROM entities WHERE name=$parent_name)
)";

const char* sql_select_account_by_name = R"(
SELECT name,password,expiration_date,flags
FROM accounts
WHERE name = ?
)";

const char* sql_select_accounts_by_multiple_names = R"(
SELECT name,password,expiration_date,flags
FROM accounts
WHERE name IN (
)";

const char* sql_select_all_accounts = R"(
SELECT name,password,expiration_date,flags
FROM accounts
)";

const char* sql_insert_account = R"(
INSERT INTO accounts (name,password,expiration_date,flags)
VALUES (?,?,?,?)
)";

const char* sql_update_account_by_name = R"(
UPDATE accounts
SET password=?,expiration_date=?,flags=?
WHERE name = ?
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

const char* sql_select_whole_hierarchy = R"(
SELECT child_id,parent_id FROM hierarchy
)";

const char* sql_select_hierarchy_root_by_child_id = R"(
WITH RECURSIVE
	hierarchy_path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM hierarchy
		WHERE child_id = $child_id
		UNION ALL
		SELECT
			hierarchy.child_id,
			hierarchy.parent_id,
			hierarchy_path.depth+1
		FROM hierarchy_path, hierarchy
		WHERE hierarchy_path.parent_id = hierarchy.child_id
		  AND hierarchy_path.child_id <> hierarchy_path.parent_id
		  AND hierarchy_path.depth < $depth
	)
SELECT DISTINCT parent_id
FROM hierarchy_path
WHERE depth IN (SELECT MAX(depth) FROM hierarchy_path)
)";

const char* sql_hierarchy_attach = R"(
INSERT INTO hierarchy(child_id,parent_id)
VALUES (?,?)
)";

const char* sql_hierarchy_detach_child_by_id = R"(
DELETE FROM hierarchy
WHERE child_id=$child_id
)";

const char* sql_hierarchy_detach_child_by_name = R"(
DELETE FROM hierarchy
WHERE child_id IN (SELECT id FROM entities WHERE name = $child_name)
)";

const char* sql_select_hierarchy_by_ids = R"(
SELECT child_id,parent_id
FROM hierarchy
WHERE child_id=$child_id AND parent_id=$parent_id
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

	struct Tie {

		sys::uid_type child_id = ggg::bad_uid;
		sys::uid_type parent_id = ggg::bad_uid;

		Tie() = default;

		Tie(sys::uid_type id1, sys::uid_type id2):
		child_id(id1), parent_id(id2) {}

		void
		dot(std::ostream& out) const {
			out << "id" << child_id << " -> " << "id" << parent_id << ";\n";
		}

	};

	void
	operator>>(const sqlite::statement& in, Tie& rhs) {
		sqlite::cstream cstr(in);
		cstr >> rhs.child_id >> rhs.parent_id;
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
	if (file == File::All) {
		this->open(File::Entities, flag);
		this->attach(File::Accounts, flag);
	} else {
		const auto& params = configurations[static_cast<int>(file)];
		sqlite::file_flag flags = sqlite::file_flag::read_only;
		if (flag == Flag::Read_write) {
			flags = sqlite::file_flag::read_write | sqlite::file_flag::create;
		}
		this->_db.open(params.filename, flags);
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
	}
	this->_db.foreign_keys(true);
	this->_db.busy_timeout(std::chrono::seconds(30));
}

void
ggg::Database::attach(File file, Flag flag) {
	const auto& params = configurations[static_cast<int>(file)];
	this->_db.attach(params.filename, params.name);
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

bool
ggg::Database::find_group(sys::gid_type gid, ggg::group& result) {
	statement_type rstr =
		this->_db.prepare(sql_select_group_by_id, gid, GGG_MAX_DEPTH);
	ggg::group::container_type members;
	ggg::group tmp;
	bool found = false;
	for (auto& tmp : rstr.rows<ggg::group>()) {
		if (tmp.id() == gid) {
			result = std::move(tmp);
			found = true;
		} else {
			members.emplace(tmp.name());
		}
	}
	result.members(std::move(members));
	return found;
}

bool
ggg::Database::find_group(const char* name, ggg::group& result) {
	statement_type rstr =
		this->_db.prepare(sql_select_group_by_name, name, GGG_MAX_DEPTH);
	ggg::group::container_type members;
	ggg::group tmp;
	bool found = false;
	for (auto& tmp : rstr.rows<ggg::group>()) {
		if (tmp.name() == name) {
			found = true;
			result = std::move(tmp);
		} else {
			members.emplace(tmp.name());
		}
	}
	result.members(std::move(members));
	return found;
}

auto
ggg::Database::find_parent_entities(const char* name) -> statement_type {
	auto id = find_id(name);
	return this->_db.prepare(sql_select_parent_entities_by_id, id, GGG_MAX_DEPTH);
}

auto
ggg::Database::find_child_entities(const char* name) -> statement_type {
	auto id = find_id(name);
	return this->_db.prepare(sql_select_child_entities_by_id, id, GGG_MAX_DEPTH);
}

auto
ggg::Database::groups() -> group_container_t {
	group_container_t groups;
	{
		auto rstr1 = this->_db.prepare(sql_select_all_groups);
		for (auto& tmp : rstr1.rows<group>()) {
			groups.emplace(tmp.id(), std::move(tmp));
		}
	}
	auto rstr2 = this->_db.prepare(sql_select_all_group_members, GGG_MAX_DEPTH);
	for (auto& tie : rstr2.rows<Tie>()) {
		auto parent = groups.find(tie.parent_id);
		if (parent == groups.end()) {
			continue;
		}
		auto child = groups.find(tie.child_id);
		if (child == groups.end()) {
			continue;
		}
		parent->second.push(child->second.name());
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
		ent.real_name(),
		home,
		shell
	);
	if (ent.has_valid_parent()) {
		auto parent_id = find_id(ent.parent().data());
		attach(id, parent_id);
	}
}

void
ggg::Database::erase(const char* name) {
	this->_db.execute(sql_delete_entity_by_name, name);
	auto nrows1 = this->_db.num_rows_modified();
	this->_db.execute("DELETE FROM accounts WHERE name=?", name);
	auto nrows2 = this->_db.num_rows_modified();
	if (nrows1 == 0 && nrows2 == 0) {
		throw std::invalid_argument("bad entity");
	}
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
	if (id == bad_uid) {
		throw std::invalid_argument("bad name");
	}
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
	return this->_db.prepare(sql_select_all_ties);
}

void
ggg::Database::dot(std::ostream& out) {
	sqlite::deferred_transaction tr(this->_db);
	out << "digraph GGG {\n";
	out << "rankdir=LR;\n";
	{
		auto rstr = entities();
		sqlite::row_iterator<entity> first(rstr), last;
		while (first != last) {
			out << "id" << first->id() << " [label=\"" << first->name() << "\"];\n";
			++first;
		}
	}
	{
		auto rstr = ties();
		sqlite::row_iterator<Tie> first(rstr), last;
		while (first != last) {
			first->dot(out);
			++first;
		}
	}
	{
		std::unordered_map<sys::uid_type,std::vector<Tie>> hierarchies;
		auto rstr = hierarchy();
		sqlite::row_iterator<Tie> first(rstr), last;
		while (first != last) {
			auto root_id = find_hierarchy_root(first->child_id);
			hierarchies[root_id].emplace_back(first->child_id, first->parent_id);
			++first;
		}
		for (const auto& entry : hierarchies) {
			auto root_id = entry.first;
			auto root_name = find_name(root_id);
			const auto& ties = entry.second;
			out << "subgraph cluster_" << root_id << " {\n";
			out << "label=\"" << root_name << "\";\n";
			for (const auto& tie : ties) {
				tie.dot(out);
			}
			out << "}\n";
		}
	}
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
	typedef std::underlying_type<account_flags>::type int_t;
	this->_db.execute(
		sql_insert_account,
		acc.name(),
		acc.password().empty() ? nullptr : acc.password().data(),
		acc.expire(),
		static_cast<int_t>(acc.flags())
	);
}

void
ggg::Database::update(const account& acc) {
	typedef std::underlying_type<account_flags>::type int_t;
	this->_db.execute(
		sql_update_account_by_name,
		acc.password().empty() ? nullptr : acc.password().data(),
		acc.expire(),
		static_cast<int_t>(acc.flags()),
		acc.name()
	);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad account");
	}
}

void
ggg::Database::set_password(const account& acc) {
	typedef std::underlying_type<account_flags>::type int_t;
	this->_db.execute(
		sql_set_password_by_name,
		acc.password().data(),
		static_cast<int_t>(account_flags::password_has_expired),
		acc.login().data()
	);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad account");
	}
}

void
ggg::Database::expire(const char* name) {
	this->_db.execute(sql_expire_account_by_name, name);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad account name");
	}
}

void
ggg::Database::set_account_flag(const char* name, account_flags flag) {
	typedef std::underlying_type<account_flags>::type int_t;
	this->_db.execute(
		sql_set_account_flag_by_name,
		static_cast<int_t>(flag),
		name
	);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad account name");
	}
}

void
ggg::Database::unset_account_flag(const char* name, account_flags flag) {
	typedef std::underlying_type<account_flags>::type int_t;
	this->_db.execute(
		sql_unset_account_flag_by_name,
		static_cast<int_t>(flag),
		name
	);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad account name");
	}
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
	Transaction tr(*this);
	if (ent.has_id() && ent.has_name()) {
		auto existing_id = find_id_nocheck(ent.name().data());
		auto existing_name = find_name_nocheck(ent.id());
		if (existing_id != bad_uid &&
			existing_id != ent.id() &&
			existing_name == ent.name()) {
			throw std::invalid_argument("changing id is not allowed");
		}
	} else if (!ent.has_id()) {
		id = find_id(ent.name().data());
	} else if (!ent.has_name()) {
		name = find_name(id);
	}
	this->_db.execute(
		sql_update_user_by_id,
		name,
		ent.real_name(),
		ent.home(),
		ent.shell(),
		id
	);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad entity");
	}
	tr.commit();
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
	typedef std::underlying_type<account_flags>::type int_t;
	int_t v = static_cast<int_t>(flag);
	return this->_db.prepare(sql_select_entities_by_flag, v, set ? v : 0);
}

auto
ggg::Database::hierarchy() -> statement_type {
	return this->_db.prepare(sql_select_whole_hierarchy);
}

sys::uid_type
ggg::Database::find_hierarchy_root(sys::uid_type child_id) {
	sys::uid_type id = bad_uid;
	auto rstr = this->_db.prepare(sql_select_hierarchy_root_by_child_id, child_id);
	if (rstr.step() != sqlite::errc::done) {
		rstr.column(0, id);
	}
	if (id == bad_uid) {
		id = child_id;
	}
	return id;
}

void
ggg::Database::detach(const char* name) {
	this->_db.execute(sql_hierarchy_detach_child_by_name, name);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad name");
	}
}

void
ggg::Database::attach(sys::uid_type child_id, sys::gid_type parent_id) {
	if (entities_are_attached(child_id, parent_id)) {
		throw std::invalid_argument("entities are attached");
	}
	if (entities_are_tied(child_id, parent_id)) {
		throw std::invalid_argument("entities are tied");
	}
	this->_db.execute(sql_hierarchy_detach_child_by_id, child_id);
	this->_db.execute(sql_hierarchy_attach, child_id, parent_id);
}

void
ggg::Database::attach(const char* child, const char* parent) {
	Transaction tr(*this);
	auto child_id = find_id(child);
	auto parent_id = find_id(parent);
	attach(child_id, parent_id);
	tr.commit();
}

void
ggg::Database::tie(sys::uid_type uid, sys::gid_type gid) {
	this->_db.execute(sql_insert_tie, uid, gid);
}

void
ggg::Database::tie(const char* child, const char* parent) {
	Transaction tr(*this);
	auto child_id = find_id(child);
	auto parent_id = find_id(parent);
	auto child_root = find_hierarchy_root(child_id);
	auto parent_root = find_hierarchy_root(parent_id);
	if (child_root == parent_root) {
		std::clog << "child_root=" << child_root << std::endl;
		std::clog << "parent_root=" << parent_root << std::endl;
		throw std::invalid_argument("same hierarchy root");
	}
	this->tie(child_id, parent_id);
	tr.commit();
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
	Transaction tr(*this);
	auto child_id = find_id(child);
	this->_db.execute(sql_delete_tie_by_id, child_id);
	tr.commit();
}

bool
ggg::Database::entities_are_tied(
	sys::uid_type child_id,
	sys::gid_type parent_id
) {
	auto rstr = this->_db.prepare(sql_select_tie_by_ids, child_id, parent_id);
	return rstr.step() != sqlite::errc::done;
}

bool
ggg::Database::entities_are_attached(
	sys::uid_type child_id,
	sys::gid_type parent_id
) {
	auto rstr = this->_db.prepare(sql_select_hierarchy_by_ids, child_id, parent_id);
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

