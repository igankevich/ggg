#ifndef GGG_CORE_SCHEMA_HH
#define GGG_CORE_SCHEMA_HH

namespace {

constexpr const int64_t entities_schema_version = 2;

constexpr const char* entities_schema[entities_schema_version] = {

R"(
CREATE TABLE entities (
         id  INTEGER  NOT NULL  PRIMARY KEY  AUTOINCREMENT,
       name  TEXT     NOT NULL  UNIQUE,
description  TEXT,
       home  TEXT,
      shell  TEXT
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
)",

R"(
CREATE TABLE hosts (
	ethernet_address  BLOB  NOT NULL  PRIMARY KEY,
	            name  TEXT  NOT NULL
);
CREATE TABLE networks (
	  name  TEXT  NOT NULL  PRIMARY KEY,
	number  BLOB  NOT NULL  UNIQUE
);
CREATE TABLE addresses (
	      ip_address  BLOB  NOT NULL  PRIMARY KEY,
	ethernet_address  BLOB  NOT NULL,
	FOREIGN KEY (ethernet_address)
		REFERENCES hosts (ethernet_address)
		ON DELETE CASCADE
		ON UPDATE CASCADE
);
CREATE INDEX host_name_index ON hosts (name);
CREATE UNIQUE INDEX ethernet_address_index ON hosts (ethernet_address);
CREATE UNIQUE INDEX network_index ON networks (number);
)"
};

constexpr const int64_t accounts_schema_version = 2;

constexpr const char* accounts_schema[accounts_schema_version] = {

R"(
CREATE TABLE accounts (
           name  TEXT      NOT NULL  PRIMARY KEY,
       password  TEXT,
expiration_date  INTEGER,
          flags  INTEGER   NOT NULL  DEFAULT 0
);
)",

R"(
CREATE TABLE messages (
    account_name TEXT NOT NULL,
    timestamp INTEGER NOT NULL,
    machine_name TEXT NOT NULL,
    message TEXT NOT NULL
);
CREATE INDEX account_name_index ON messages (account_name);
CREATE INDEX timestamp_index ON messages (timestamp);
)"

};

}

#endif // vim:filetype=cpp
