#include <type_traits>

#include <ggg/config.hh>
#include <ggg/core/database.hh>

#include <iostream>

namespace {

const int64_t schema_version = 1;

const char* sql_schema = R"(
CREATE TABLE entities (
	id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	name TEXT NOT NULL UNIQUE,
	description TEXT,
	home TEXT,
	shell TEXT,
	password TEXT,
	expiration_date INTEGER,
	flags INTEGER
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

const char* sql_insert_user = R"(
INSERT INTO entities
	(id,name,description,home,shell)
VALUES (?,?,?,?,?)
)";

const char* sql_select_id_by_name = R"(
SELECT id
FROM entities
WHERE name = ?
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

const char* sql_insert_tie = R"(
INSERT INTO ties (child_id,parent_id) VALUES (?,?)
)";

const char* sql_select_account_by_name = R"(
SELECT name,password,expiration_date,flags
FROM entities
WHERE name = ?
)";

const char* sql_select_all_accounts = R"(
SELECT name,password,expiration_date,flags
FROM entities
)";

const char* sql_update_account_by_name = R"(
UPDATE entities
SET password=?,expiration_date=?,flags=?
WHERE name = ?
)";

	struct Tie {

		int64_t child_id;
		int64_t parent_id;

	};

	sqlite::rstream&
	operator>>(sqlite::rstream& in, Tie& rhs) {
		sqlite::cstream cstr(in);
		if (in >> cstr) {
			cstr >> rhs.child_id >> rhs.parent_id;
		}
		return in;
	}

}

void
ggg::Database::open(const char* filename, bool read) {
	this->close();
	this->_readonly = read;
	this->_db.open(
		filename,
		read
		? sqlite::open_flag::read_only
		: (sqlite::open_flag::read_write | sqlite::open_flag::create)
	);
	int64_t version = this->_db.user_version();
	if (version < schema_version) {
		this->_db.execute(sql_schema);
		this->_db.user_version(schema_version);
	}
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

void
ggg::Database::insert(const entity& ent) {
	if (!(ent.id() >= GGG_MIN_ID)) {
		throw std::invalid_argument("bad uid/gid");
	}
	if (!ent.has_valid_name()) {
		throw std::invalid_argument("bad name");
	}
//	if (struct ::passwd* pw = ::getpwnam(ent.name().data())) {
//		if (pw->pw_uid < GGG_MIN_UID || pw->pw_gid < GGG_MIN_GID) {
//			throw std::invalid_argument("conflicting system user");
//		}
//	}
	this->_db.execute(
		sql_insert_user,
		ent.id(),
		ent.name(),
		ent.real_name(),
		ent.home().empty() ? nullptr : ent.home().data(),
		ent.shell().empty() ? nullptr : ent.shell().data()
	);
}

void
ggg::Database::tie(sys::uid_type uid, sys::gid_type gid) {
	this->_db.execute(sql_insert_tie, uid, gid);
}

sys::uid_type
ggg::Database::find_id(const char* name) {
	int64_t id = -1;
	auto rstr = this->_db.prepare(sql_select_id_by_name, name);
	sqlite::cstream cstr(rstr);
	if (rstr >> cstr) {
		cstr >> id;
	}
	return static_cast<sys::uid_type>(id);
}

auto
ggg::Database::ties() -> row_stream_t {
	return this->_db.prepare(sql_select_all_ties);
}

void
ggg::Database::dot(std::ostream& out) {
	out << "digraph {\n";
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
			out << "id" << first->child_id
				<< " -> "
				<< "id" << first->parent_id
				<< ";\n";
			++first;
		}
	}
	out << "}\n";
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
		std::clog << "bad account: " << acc << std::endl;
	}
}

