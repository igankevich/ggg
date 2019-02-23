#include <ggg/test/clean_database.hh>
#include <grp.h>
#include <gtest/gtest.h>

struct gr_guard {
	gr_guard() { ::setgrent(); }
	~gr_guard() { ::endgrent(); }
};

TEST(group, empty) {
	{ Clean_database db; }
	gr_guard g;
	struct ::group* ent = ::getgrent();
	ASSERT_EQ(nullptr, ent) << ent->gr_name;
}

TEST(group, getgrent) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
	}
	gr_guard g;
	struct ::group* ent = ::getgrent();
	ASSERT_NE(nullptr, ent);
	EXPECT_STREQ("testuser", ent->gr_name);
}

TEST(group, getgrnam) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
		db.insert("user2:x:2001:2001:halt:/sbin:/sbin/halt");
		db.insert("user3:x:2002:2002:halt:/sbin:/sbin/halt");
	}
	struct group* user1 = getgrnam("testuser");
	ASSERT_NE(nullptr, user1);
	EXPECT_STREQ("testuser", user1->gr_name);
	EXPECT_EQ(2000, user1->gr_gid);
	struct group* user2 = getgrnam("user2");
	EXPECT_STREQ("user2", user2->gr_name);
	EXPECT_EQ(2001, user2->gr_gid);
	ASSERT_NE(nullptr, user2);
	struct group* user3 = getgrnam("user3");
	ASSERT_NE(nullptr, user3);
	EXPECT_STREQ("user3", user3->gr_name);
	EXPECT_EQ(2002, user3->gr_gid);
	struct group* user4 = getgrnam("nonexistent");
	ASSERT_EQ(nullptr, user4);
}

TEST(group, getgrgid) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
		db.insert("user2:x:2001:2001:halt:/sbin:/sbin/halt");
		db.insert("user3:x:2002:2002:halt:/sbin:/sbin/halt");
	}
	struct group* user1 = getgrgid(2000);
	ASSERT_NE(nullptr, user1);
	EXPECT_STREQ("testuser", user1->gr_name);
	EXPECT_EQ(2000, user1->gr_gid);
	struct group* user2 = getgrgid(2001);
	EXPECT_STREQ("user2", user2->gr_name);
	EXPECT_EQ(2001, user2->gr_gid);
	ASSERT_NE(nullptr, user2);
	struct group* user3 = getgrgid(2002);
	ASSERT_NE(nullptr, user3);
	EXPECT_STREQ("user3", user3->gr_name);
	EXPECT_EQ(2002, user3->gr_gid);
	struct group* user4 = getgrgid(12345);
	ASSERT_EQ(nullptr, user4);
}

TEST(group, getgrouplist) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
	}
	int ngroups = 4096 / sizeof(::gid_t);
	::gid_t groups[ngroups];
	int ret = ::getgrouplist("testuser", 2000, groups, &ngroups);
	ASSERT_EQ(1, ret);
}

TEST(group, group_member_via_tie) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
		db.insert("testgroup:x:2001:2001:halt:/sbin:/sbin/halt");
		db.tie("testuser", "testgroup");
	}
	auto* testgroup = getgrnam("testgroup");
	EXPECT_STREQ("testuser", *testgroup->gr_mem);
	int ngroups = 4096 / sizeof(::gid_t);
	::gid_t groups[ngroups];
	int ret = ::getgrouplist("testuser", 2000, groups, &ngroups);
	if (ret > 0 && ret != 2) {
		for (int i=0; i<ret; ++i) {
			std::clog << "groups[i]=" << groups[i] << std::endl;
		}
	}
	ASSERT_EQ(2, ret);
	if (groups[0] == 2000) {
		EXPECT_EQ(2001, groups[1]);
	} else if (groups[0] == 2001) {
		EXPECT_EQ(2000, groups[1]);
	} else {
		FAIL() << "groups=" << groups[0] << ',' << groups[1];
	}
}

TEST(group, getgrent_nested_groups) {
	{
		Clean_database db;
		db.insert("u1:x:2000:2000:halt:/sbin:/sbin/halt");
		db.insert("g1:x:2001:2001:halt:/sbin:/sbin/halt");
		db.insert("g2:x:2002:2002:halt:/sbin:/sbin/halt");
		db.tie("u1", "g1");
		db.tie("g1", "g2");
	}
	gr_guard g;
	while (const auto* ent = ::getgrent()) {
		std::string name = ent->gr_name;
		if (name == "u1") {
			ASSERT_NE(nullptr, ent->gr_mem);
			EXPECT_EQ(nullptr, ent->gr_mem[0]) << ent->gr_mem[0];
		} else if (name == "g1") {
			ASSERT_NE(nullptr, ent->gr_mem);
			ASSERT_NE(nullptr, ent->gr_mem[0]) << ent->gr_mem[0];
			ASSERT_EQ(nullptr, ent->gr_mem[1]) << ent->gr_mem[1];
			EXPECT_STREQ("u1", ent->gr_mem[0]);
		} else if (name == "g2") {
			ASSERT_NE(nullptr, ent->gr_mem);
			ASSERT_NE(nullptr, ent->gr_mem[0]) << ent->gr_mem[0];
			ASSERT_NE(nullptr, ent->gr_mem[1]) << ent->gr_mem[1];
			ASSERT_EQ(nullptr, ent->gr_mem[2]) << ent->gr_mem[2];
			if (ent->gr_mem[0] && ent->gr_mem[1]) {
				std::string m1 = ent->gr_mem[0];
				std::string m2 = ent->gr_mem[1];
				if (m1 < m2) {
					std::swap(m1, m2);
				}
				EXPECT_EQ("u1", m1);
				EXPECT_EQ("g1", m2);
			}
		} else {
			FAIL() << "unexpected group: " << name;
		}
	}
}
