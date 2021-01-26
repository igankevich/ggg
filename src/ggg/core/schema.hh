#ifndef GGG_CORE_SCHEMA_HH
#define GGG_CORE_SCHEMA_HH

namespace {

constexpr const int64_t entities_schema_version = 3;

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
)",

R"(
-- 0 user->user edge
-- 1 user->group edge
-- 2 group->group edge
ALTER TABLE ties ADD COLUMN type INTEGER NOT NULL DEFAULT 0;
CREATE INDEX ties_type_index ON ties (type);
UPDATE ties SET type=1;
INSERT INTO ties (child_id,parent_id,type) SELECT child_id,parent_id,0 FROM hierarchy;
DROP INDEX hierarchy_index;
DELETE FROM hierarchy;
)",

    /*
R"(
CREATE TABLE users (
    id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT,
    home TEXT,
    shell TEXT
);
CREATE UNIQUE INDEX users_name_index ON users (name);
-- start IDs from 1000
INSERT INTO sqlite_sequence (name,seq) VALUES ('users',999);

CREATE TABLE users_hierarchy (
    child_id INTEGER NOT NULL,
    parent_id INTEGER NOT NULL,
    PRIMARY KEY (child_id, parent_id),
    FOREIGN KEY (child_id)
        REFERENCES users (id)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    FOREIGN KEY (parent_id)
        REFERENCES users (id)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
);
CREATE UNIQUE INDEX users_hierarchy_index ON users_hierarchy (child_id,parent_id);

CREATE TABLE groups (
    id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    description TEXT
);
CREATE UNIQUE INDEX groups_name_index ON groups (name);
-- start IDs from 1000
INSERT INTO sqlite_sequence (name,seq) VALUES ('groups',999);

CREATE TABLE groups_hierarchy (
    child_id INTEGER NOT NULL,
    parent_id INTEGER NOT NULL,
    PRIMARY KEY (child_id, parent_id),
    FOREIGN KEY (child_id)
        REFERENCES groups (id)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    FOREIGN KEY (parent_id)
        REFERENCES groups (id)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
);
CREATE UNIQUE INDEX groups_hierarchy_index ON groups_hierarchy (child_id,parent_id);

CREATE TABLE user_group_ties (
    user_id INTEGER NOT NULL,
    group_id INTEGER NOT NULL,
    PRIMARY KEY (user_id, group_id),
    FOREIGN KEY (user_id)
        REFERENCES users (id)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
    FOREIGN KEY (group_id)
        REFERENCES groups (id)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
);

CREATE UNIQUE INDEX user_group_ties_index ON user_group_ties (user_id,group_id);

)"
*/

};

constexpr const int64_t accounts_schema_version = 4;

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
CREATE INDEX messages_account_name_index ON messages (account_name);
CREATE INDEX messages_timestamp_index ON messages (timestamp);
)",

R"(
CREATE TABLE public_keys (
    account_name TEXT NOT NULL,
    options TEXT,
    type TEXT NOT NULL,
    key TEXT NOT NULL,
    comment TEXT,
    FOREIGN KEY (account_name)
        REFERENCES accounts (name)
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    UNIQUE (account_name,key)
);
CREATE INDEX public_keys_account_name_index ON public_keys (account_name);
)",

R"(
ALTER TABLE accounts ADD COLUMN max_inactive INTEGER;
ALTER TABLE accounts ADD COLUMN last_active INTEGER;
)"

};

}

#endif // vim:filetype=cpp
