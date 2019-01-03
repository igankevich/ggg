#include <ggg/nss/pw_guard.hh>
#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>

TEST(pwd, Empty) {
	{ Clean_database db; }
	pw_guard g;
	struct ::passwd* ent = ::getpwent();
	ASSERT_EQ(nullptr, ent) << ent->pw_name;
}

TEST(pwd, getpwent) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
	}
	pw_guard g;
	struct ::passwd* ent = ::getpwent();
	ASSERT_NE(nullptr, ent);
	EXPECT_STREQ("testuser", ent->pw_name);
}

TEST(pwd, getpwnam) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
		db.insert("user2:x:2001:2001:halt:/sbin:/sbin/halt");
		db.insert("user3:x:2002:2002:halt:/sbin:/sbin/halt");
	}
	struct passwd* user1 = getpwnam("testuser");
	ASSERT_NE(nullptr, user1);
	EXPECT_STREQ("testuser", user1->pw_name);
	EXPECT_EQ(2000, user1->pw_uid);
	struct passwd* user2 = getpwnam("user2");
	EXPECT_STREQ("user2", user2->pw_name);
	EXPECT_EQ(2001, user2->pw_uid);
	ASSERT_NE(nullptr, user2);
	struct passwd* user3 = getpwnam("user3");
	ASSERT_NE(nullptr, user3);
	EXPECT_STREQ("user3", user3->pw_name);
	EXPECT_EQ(2002, user3->pw_uid);
	struct passwd* user4 = getpwnam("nonexistent");
	ASSERT_EQ(nullptr, user4);
}

TEST(pwd, getpwuid) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
		db.insert("user2:x:2001:2001:halt:/sbin:/sbin/halt");
		db.insert("user3:x:2002:2002:halt:/sbin:/sbin/halt");
	}
	struct passwd* user1 = getpwuid(2000);
	ASSERT_NE(nullptr, user1);
	EXPECT_STREQ("testuser", user1->pw_name);
	EXPECT_EQ(2000, user1->pw_uid);
	struct passwd* user2 = getpwuid(2001);
	EXPECT_STREQ("user2", user2->pw_name);
	EXPECT_EQ(2001, user2->pw_uid);
	ASSERT_NE(nullptr, user2);
	struct passwd* user3 = getpwuid(2002);
	ASSERT_NE(nullptr, user3);
	EXPECT_STREQ("user3", user3->pw_name);
	EXPECT_EQ(2002, user3->pw_uid);
	struct passwd* user4 = getpwuid(12345);
	ASSERT_EQ(nullptr, user4);
}
