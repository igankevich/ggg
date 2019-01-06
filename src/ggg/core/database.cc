#include <limits>
#include <regex>
#include <type_traits>
#include <vector>

#include <ggg/bits/to_bytes.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>

#include <iostream>

namespace {

const int64_t entities_schema_version = 1;

const char* sql_entities_schema = R"(
CREATE TABLE entities (
	id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL UNIQUE,
	description TEXT,
	home TEXT,
	shell TEXT
);

CREATE UNIQUE INDEX entities_name_index ON entities (name);

-- start IDs from 1000
INSERT INTO sqlite_sequence (name,seq) VALUES ('entities',999);

CREATE TABLE ties (
	child_id INTEGER NOT NULL,
	parent_id INTEGER NOT NULL,
	PRIMARY KEY (child_id, parent_id),
	FOREIGN KEY (child_id)
		REFERENCES entities (id)
		ON DELETE CASCADE
		ON UPDATE RESTRICT,
	FOREIGN KEY (parent_id)
		REFERENCES entities (id)
		ON DELETE CASCADE
		ON UPDATE RESTRICT
);

CREATE UNIQUE INDEX ties_index ON ties (child_id,parent_id);

CREATE TABLE hierarchy (
	child_id INTEGER NOT NULL,
	parent_id INTEGER NOT NULL,
	PRIMARY KEY (child_id, parent_id),
	FOREIGN KEY (child_id)
		REFERENCES entities (id)
		ON DELETE CASCADE
		ON UPDATE RESTRICT,
	FOREIGN KEY (parent_id)
		REFERENCES entities (id)
		ON DELETE CASCADE
		ON UPDATE RESTRICT
);

CREATE UNIQUE INDEX hierarchy_index ON hierarchy (child_id,parent_id);
)";

const int64_t accounts_schema_version = 1;

const char* sql_accounts_schema = R"(
CREATE TABLE accounts (
	name TEXT NOT NULL PRIMARY KEY,
	password TEXT,
	expiration_date INTEGER,
	flags INTEGER NOT NULL DEFAULT 0
);
)";

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
	-- find all entities higher in the hierarchy
	hierarchy_path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM hierarchy
		WHERE child_id = $id
		UNION ALL
		SELECT hierarchy.child_id, hierarchy.parent_id, hierarchy_path.depth+1
		FROM hierarchy_path, hierarchy
		WHERE hierarchy_path.parent_id = hierarchy.child_id
		  AND hierarchy_path.child_id <> hierarchy_path.parent_id
		  AND hierarchy_path.depth < $depth
	),
	-- remove duplicate entities (include this entity in case there is no
	-- entity higher in the hierarchy)
	unique_hierarchy_path(id) AS (
		SELECT $id
		UNION
		SELECT DISTINCT child_id FROM hierarchy_path
		UNION
		SELECT DISTINCT parent_id FROM hierarchy_path
	),
	-- find all ties between entities in the hiearachy and other entities
	ties_subgraph(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1
		FROM ties
		WHERE parent_id IN (SELECT id FROM unique_hierarchy_path)
		UNION ALL
		SELECT ties.child_id, ties.parent_id, ties_subgraph.depth+1
		FROM ties_subgraph, ties
		WHERE ties_subgraph.child_id = ties.parent_id
		  AND ties_subgraph.child_id <> ties_subgraph.parent_id
		  AND ties_subgraph.depth < $depth
	),
	-- remove duplicate entities
	unique_ids(id) AS (
		SELECT $id
		UNION
		SELECT DISTINCT child_id FROM ties_subgraph
		UNION
		SELECT DISTINCT parent_id FROM ties_subgraph
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT id FROM unique_ids)
)";

const char* sql_select_group_by_name = R"(
WITH RECURSIVE
	-- find group id by group name
	group_ids(id) AS (
		SELECT id FROM entities WHERE name = $name
	),
	-- find all entities higher in the hierarchy
	hierarchy_path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM hierarchy
		WHERE child_id IN (SELECT id FROM group_ids)
		UNION ALL
		SELECT hierarchy.child_id, hierarchy.parent_id, hierarchy_path.depth+1
		FROM hierarchy_path, hierarchy
		WHERE hierarchy_path.parent_id = hierarchy.child_id
		  AND hierarchy_path.child_id <> hierarchy_path.parent_id
		  AND hierarchy_path.depth < $depth
	),
	-- remove duplicate entities (include this entity in case there is no
	-- entity higher in the hierarchy)
	unique_hierarchy_path(id) AS (
		SELECT id FROM group_ids
		UNION
		SELECT DISTINCT child_id FROM hierarchy_path
		UNION
		SELECT DISTINCT parent_id FROM hierarchy_path
	),
	-- find all ties between entities in the hiearachy and other entities
	ties_subgraph(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1
		FROM ties
		WHERE parent_id IN (SELECT id FROM unique_hierarchy_path)
		UNION ALL
		SELECT ties.child_id, ties.parent_id, ties_subgraph.depth+1
		FROM ties_subgraph, ties
		WHERE ties_subgraph.child_id = ties.parent_id
		  AND ties_subgraph.child_id <> ties_subgraph.parent_id
		  AND ties_subgraph.depth < $depth
	),
	-- remove duplicate entities
	unique_ids(id) AS (
		SELECT id FROM group_ids
		UNION
		SELECT DISTINCT child_id FROM ties_subgraph
		UNION
		SELECT DISTINCT parent_id FROM ties_subgraph
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT id FROM unique_ids)
)";

const char* sql_select_parent_entities_by_name = R"(
WITH RECURSIVE
	-- find entity id by entity name
	entity_ids(id) AS (
		SELECT id FROM entities WHERE name = $name
	),
	-- find all entities higher in the hierarchy
	hierarchy_path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM hierarchy
		WHERE child_id IN (SELECT id FROM entity_ids)
		UNION ALL
		SELECT hierarchy.child_id, hierarchy.parent_id, hierarchy_path.depth+1
		FROM hierarchy_path, hierarchy
		WHERE hierarchy_path.parent_id = hierarchy.child_id
		  AND hierarchy_path.child_id <> hierarchy_path.parent_id
		  AND hierarchy_path.depth < $depth
	),
	-- remove duplicate entities (include this entity in case there is no
	-- entity higher in the hierarchy)
	unique_hierarchy_path(id) AS (
		SELECT id FROM entity_ids
		UNION
		SELECT DISTINCT child_id FROM hierarchy_path
		UNION
		SELECT DISTINCT parent_id FROM hierarchy_path
	),
	-- find all ties between entities in the hiearachy and other entities
	ties_subgraph(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1
		FROM ties
		WHERE child_id IN (SELECT id FROM unique_hierarchy_path)
		UNION ALL
		SELECT ties.child_id, ties.parent_id, ties_subgraph.depth+1
		FROM ties_subgraph, ties
		WHERE ties_subgraph.parent_id = ties.child_id
		  AND ties_subgraph.child_id <> ties_subgraph.parent_id
		  AND ties_subgraph.depth < $depth
	),
	-- remove duplicate entities
	unique_ids(id) AS (
		SELECT DISTINCT child_id FROM hierarchy_path
		UNION
		SELECT DISTINCT parent_id FROM hierarchy_path
		UNION
		SELECT DISTINCT child_id FROM ties_subgraph
		UNION
		SELECT DISTINCT parent_id FROM ties_subgraph
	),
	clean_ids(id) AS (
		SELECT id
		FROM unique_ids
		WHERE id NOT IN (SELECT id FROM entity_ids)
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT id FROM clean_ids)
)";

const char* sql_select_child_entities_by_name = R"(
WITH RECURSIVE
	-- find entity id by entity name
	entity_ids(id) AS (
		SELECT id FROM entities WHERE name = $name
	),
	-- find all entities lower in the hierarchy
	hierarchy_path(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1 FROM hierarchy
		WHERE parent_id IN (SELECT id FROM entity_ids)
		UNION ALL
		SELECT hierarchy.child_id, hierarchy.parent_id, hierarchy_path.depth+1
		FROM hierarchy_path, hierarchy
		WHERE hierarchy_path.child_id = hierarchy.parent_id
		  AND hierarchy_path.child_id <> hierarchy_path.parent_id
		  AND hierarchy_path.depth < $depth
	),
	-- remove duplicate entities (include this entity in case there is no
	-- entity higher in the hierarchy)
	unique_hierarchy_path(id) AS (
		SELECT id FROM entity_ids
		UNION
		SELECT DISTINCT child_id FROM hierarchy_path
		UNION
		SELECT DISTINCT parent_id FROM hierarchy_path
	),
	-- find all ties between entities in the hiearachy and other entities
	ties_subgraph(child_id,parent_id,depth) AS (
		SELECT child_id,parent_id,1
		FROM ties
		WHERE parent_id IN (SELECT id FROM unique_hierarchy_path)
		UNION ALL
		SELECT ties.child_id, ties.parent_id, ties_subgraph.depth+1
		FROM ties_subgraph, ties
		WHERE ties_subgraph.child_id = ties.parent_id
		  AND ties_subgraph.child_id <> ties_subgraph.parent_id
		  AND ties_subgraph.depth < $depth
	),
	-- remove duplicate entities
	unique_ids(id) AS (
		SELECT DISTINCT child_id FROM hierarchy_path
		UNION
		SELECT DISTINCT parent_id FROM hierarchy_path
		UNION
		SELECT DISTINCT child_id FROM ties_subgraph
		UNION
		SELECT DISTINCT parent_id FROM ties_subgraph
	),
	clean_ids(id) AS (
		SELECT id
		FROM unique_ids
		WHERE id NOT IN (SELECT id FROM entity_ids)
	)
SELECT id,name,description
FROM entities
WHERE id IN (SELECT id FROM clean_ids)
)";

const char* sql_select_all_group_members = R"(
WITH RECURSIVE
	hierarchy_path(child_id,parent_id,depth,initial_id) AS (
		SELECT child_id,parent_id,1,child_id FROM hierarchy
		UNION ALL
		SELECT
			hierarchy.child_id,
			hierarchy.parent_id,
			hierarchy_path.depth+1,
			hierarchy_path.initial_id
		FROM hierarchy_path, hierarchy
		WHERE hierarchy_path.parent_id = hierarchy.child_id
		  AND hierarchy_path.child_id <> hierarchy_path.parent_id
		  AND hierarchy_path.depth < $depth
	),
	ties_subgraph(child_id,parent_id,depth,initial_id) AS (
		SELECT child_id,parent_id,1,parent_id FROM ties
		UNION ALL
		SELECT
			ties.child_id,
			ties.parent_id,
			ties_subgraph.depth+1,
			ties_subgraph.initial_id
		FROM ties_subgraph, ties
		WHERE ties_subgraph.child_id = ties.parent_id
		  AND ties_subgraph.child_id <> ties_subgraph.parent_id
		  AND ties_subgraph.depth < $depth
	),
	unique_ids(parent_id,child_id) AS (
		SELECT DISTINCT initial_id,child_id FROM ties_subgraph
		UNION
		SELECT DISTINCT initial_id,parent_id
		FROM ties_subgraph
		WHERE parent_id <> initial_id
		UNION
		SELECT DISTINCT initial_id,child_id FROM hierarchy_path
		WHERE child_id <> initial_id
		UNION
		SELECT DISTINCT initial_id,parent_id FROM hierarchy_path
	)
SELECT child_id,parent_id FROM unique_ids
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

const char* sql_delete_tie_by_child_name = R"(
DELETE FROM ties
WHERE child_id IN (SELECT id FROM entities WHERE name=$child_name)
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

const char* sql_hierarchy_detach_child_by_name = R"(
DELETE FROM hierarchy
WHERE child_id IN (SELECT id FROM entities WHERE name = $child_name)
)";

const char* sql_select_hierarchy_by_ids = R"(
SELECT child_id,parent_id
FROM hierarchy
WHERE child_id=$child_id AND parent_id=$parent_id
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

	sqlite::rstream&
	operator>>(sqlite::rstream& in, Tie& rhs) {
		sqlite::cstream cstr(in);
		if (in >> cstr) {
			cstr >> rhs.child_id >> rhs.parent_id;
		}
		return in;
	}

	struct database_parameters {
		const char* filename;
		const char* schema;
		const int64_t schema_version;
		const char* name;
	};

	const database_parameters configurations[] = {
		{
			GGG_ENTITIES_PATH,
			sql_entities_schema,
			entities_schema_version,
			"entities"
		},
		{
			GGG_ACCOUNTS_PATH,
			sql_accounts_schema,
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
		sqlite::open_flag flags = sqlite::open_flag::read_only;
		if (flag == Flag::Read_write) {
			flags = sqlite::open_flag::read_write | sqlite::open_flag::create;
		}
		this->_db.open(params.filename, flags);
		int64_t version = this->_db.user_version();
		if (version < params.schema_version) {
			if (flag == Flag::Read_only) {
				throw std::invalid_argument(
					"unable to update schema of read-only database"
				);
			}
			this->_db.execute(params.schema);
			this->_db.user_version(params.schema_version);
		}
	}
	this->_db.enable_foreign_keys();
	this->_db.busy_timeout(std::chrono::seconds(30));
}

void
ggg::Database::attach(File file, Flag flag) {
	const auto& params = configurations[static_cast<int>(file)];
	this->_db.attach(params.filename, params.name);
}

auto
ggg::Database::find_entity(int64_t id) -> row_stream_t {
	return this->_db.prepare(sql_select_user_by_id, id);
}

auto
ggg::Database::find_entity(const char* name) -> row_stream_t {
	return this->_db.prepare(sql_select_user_by_name, name);
}

auto
ggg::Database::entities() -> row_stream_t {
	return this->_db.prepare(sql_select_all_users);
}

auto
ggg::Database::search_entities() -> row_stream_t {
	return this->_db.prepare(sql_search_entities);
}

bool
ggg::Database::find_group(sys::gid_type gid, ggg::group& result) {
	row_stream_t rstr =
		this->_db.prepare(sql_select_group_by_id, gid, GGG_MAX_DEPTH);
	ggg::group::container_type members;
	ggg::group tmp;
	bool found = false;
	while (rstr >> tmp) {
		if (tmp.id() == gid) {
			result = std::move(tmp);
			found = true;
		} else {
			members.emplace(tmp.name());
		}
	}
	result.members(std::move(members));
	return found && !rstr.fail() && !rstr.bad();
}

bool
ggg::Database::find_group(const char* name, ggg::group& result) {
	row_stream_t rstr =
		this->_db.prepare(sql_select_group_by_name, name, GGG_MAX_DEPTH);
	ggg::group::container_type members;
	ggg::group tmp;
	bool found = false;
	while (rstr >> tmp) {
		if (tmp.name() == name) {
			found = true;
			result = std::move(tmp);
		} else {
			members.emplace(tmp.name());
		}
	}
	result.members(std::move(members));
	return found && !rstr.fail() && !rstr.bad();
}

auto
ggg::Database::find_parent_entities(const char* name) -> row_stream_t {
	if (!this->contains(name)) {
		throw std::invalid_argument("bad entity");
	}
	return this->_db.prepare(
		sql_select_parent_entities_by_name,
		name,
		GGG_MAX_DEPTH
	);
}

auto
ggg::Database::find_child_entities(const char* name) -> row_stream_t {
	if (!this->contains(name)) {
		throw std::invalid_argument("bad entity");
	}
	return this->_db.prepare(
		sql_select_child_entities_by_name,
		name,
		GGG_MAX_DEPTH
	);
}

auto
ggg::Database::groups() -> group_container_t {
	group_container_t groups;
	{
		auto rstr1 = this->_db.prepare(sql_select_all_groups);
		ggg::group tmp;
		while (rstr1 >> tmp) {
			groups.emplace(tmp.id(), std::move(tmp));
		}
	}
	auto rstr2 = this->_db.prepare(sql_select_all_group_members, GGG_MAX_DEPTH);
	Tie tie;
	while (rstr2 >> tie) {
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
	sqlite::cstream cstr(rstr);
	if (rstr >> cstr) {
		cstr >> id;
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
ggg::Database::insert(const entity& ent) {
	if (!ent.has_valid_name()) {
		throw std::invalid_argument("bad name");
	}
	if (struct ::passwd* pw = ::getpwnam(ent.name().data())) {
		if (pw->pw_uid < GGG_MIN_ID || pw->pw_gid < GGG_MIN_ID) {
			throw std::invalid_argument("conflicting system user");
		}
	}
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
}

void
ggg::Database::erase(const char* name) {
	this->_db.execute(sql_delete_entity_by_name, name);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad entity");
	}
}

sys::uid_type
ggg::Database::find_id(const char* name) {
	sys::uid_type id = bad_uid;
	auto rstr = this->_db.prepare(sql_select_id_by_name, name);
	sqlite::cstream cstr(rstr);
	if (rstr >> cstr) {
		cstr >> id;
	}
	if (id == bad_uid) {
		throw std::invalid_argument("bad name");
	}
	return id;
}

std::string
ggg::Database::find_name(sys::uid_type id) {
	std::string name;
	auto rstr = this->_db.prepare(sql_select_name_by_id, id);
	sqlite::cstream cstr(rstr);
	if (rstr >> cstr) {
		cstr >> name;
	}
	return name;
}

auto
ggg::Database::ties() -> row_stream_t {
	return this->_db.prepare(sql_select_all_ties);
}

void
ggg::Database::dot(std::ostream& out) {
	sqlite::deferred_transaction tr(this->_db);
	out << "digraph GGG {\n";
	out << "rankdir=LR;\n";
	{
		auto rstr = entities();
		sqlite::rstream_iterator<entity> first(rstr), last;
		while (first != last) {
			out << "id" << first->id() << " [label=\"" << first->name() << "\"];\n";
			++first;
		}
	}
	{
		auto rstr = ties();
		sqlite::rstream_iterator<Tie> first(rstr), last;
		while (first != last) {
			first->dot(out);
			++first;
		}
	}
	{
		std::unordered_map<sys::uid_type,std::vector<Tie>> hierarchies;
		auto rstr = hierarchy();
		sqlite::rstream_iterator<Tie> first(rstr), last;
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
ggg::Database::find_account(const char* name) -> row_stream_t {
	return this->_db.prepare(sql_select_account_by_name, name);
}

auto
ggg::Database::accounts() -> row_stream_t {
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
	this->_db.execute(
		sql_update_user_by_id,
		ent.name(),
		ent.real_name(),
		ent.home(),
		ent.shell(),
		ent.id()
	);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad entity");
	}
}

auto
ggg::Database::expired_entities() -> row_stream_t {
	return this->_db.prepare(sql_select_expired_entities);
}

auto
ggg::Database::expired_ids() -> row_stream_t {
	return this->_db.prepare(sql_select_expired_ids);
}

auto
ggg::Database::expired_names() -> row_stream_t {
	return this->_db.prepare(sql_select_expired_names);
}

auto
ggg::Database::find_entities_by_flag(
	account_flags flag,
	bool set
) -> row_stream_t {
	typedef std::underlying_type<account_flags>::type int_t;
	int_t v = static_cast<int_t>(flag);
	return this->_db.prepare(sql_select_entities_by_flag, v, set ? v : 0);
}

auto
ggg::Database::hierarchy() -> row_stream_t {
	return this->_db.prepare(sql_select_whole_hierarchy);
}

sys::uid_type
ggg::Database::find_hierarchy_root(sys::uid_type child_id) {
	sys::uid_type id = bad_uid;
	auto rstr =
		this->_db.prepare(sql_select_hierarchy_root_by_child_id, child_id);
	sqlite::cstream cstr(rstr);
	if (rstr >> cstr) {
		cstr >> id;
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
ggg::Database::attach(const char* child, const char* parent) {
	Transaction tr(*this);
	auto child_id = find_id(child);
	auto parent_id = find_id(parent);
	if (entities_are_attached(child_id, parent_id)) {
		throw std::invalid_argument("entities are attached");
	}
	if (entities_are_tied(child_id, parent_id)) {
		throw std::invalid_argument("entities are tied");
	}
	this->_db.execute(sql_hierarchy_detach_child_by_name, child);
	this->_db.execute(sql_hierarchy_attach, child_id, parent_id);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad names");
	}
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
		throw std::invalid_argument("same hierarchy root");
	}
	this->tie(child_id, parent_id);
	tr.commit();
}

void
ggg::Database::untie(const char* child) {
	this->_db.execute(sql_delete_tie_by_child_name, child);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad name");
	}
}

void
ggg::Database::untie(const char* child, const char* parent) {
	this->_db.execute(sql_delete_tie_by_child_and_parent_name, child, parent);
	if (this->_db.num_rows_modified() == 0) {
		throw std::invalid_argument("bad names");
	}
}

bool
ggg::Database::entities_are_tied(
	sys::uid_type child_id,
	sys::gid_type parent_id
) {
	auto rstr = this->_db.prepare(sql_select_tie_by_ids, child_id, parent_id);
	rstr.step();
	return !rstr.eof();
}

bool
ggg::Database::entities_are_attached(
	sys::uid_type child_id,
	sys::gid_type parent_id
) {
	auto rstr = this->_db.prepare(sql_select_hierarchy_by_ids, child_id, parent_id);
	rstr.step();
	return !rstr.eof();
}

