#include <ggg/nss/gr_guard.hh>
#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>
#include <sstream>

class GrpTest: public ::testing::Test {};

TEST(grp, Empty) {
	{ Clean_database db; }
	gr_guard g;
	struct ::group* ent = ::getgrent();
	ASSERT_EQ(nullptr, ent) << ent->gr_name;
}

TEST(grp, getgrent) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
	}
	gr_guard g;
	struct ::group* ent = ::getgrent();
	ASSERT_NE(nullptr, ent);
	EXPECT_STREQ("testuser", ent->gr_name);
}

TEST(grd, getgrnam) {
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

TEST(grp, getgrgid) {
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

TEST(grp, getgrouplist) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
	}
	int ngroups = 4096 / sizeof(::gid_t);
	::gid_t groups[ngroups];
	int ret = ::getgrouplist("testuser", 2000, groups, &ngroups);
	ASSERT_EQ(1, ret);
}
