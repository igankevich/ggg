#include <ggg/nss/entity_traits.hh>
#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>
#include <pwd.h>

struct pw_guard {
	pw_guard() { ::setpwent(); }
	~pw_guard() { ::endpwent(); }
};

TEST(passwd, empty) {
	{ Clean_database db; }
	pw_guard g;
	struct ::passwd* ent = ::getpwent();
	ASSERT_EQ(nullptr, ent) << ent->pw_name;
}

TEST(passwd, getpwent) {
	{
		Clean_database db;
		db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
	}
	pw_guard g;
	struct ::passwd* ent = ::getpwent();
	ASSERT_NE(nullptr, ent);
	EXPECT_STREQ("testuser", ent->pw_name);
}

TEST(passwd, getpwnam) {
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

TEST(passwd, getpwuid) {
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

TEST(passwd, copy) {
	ggg::entity ent;
	char fillchar = -1;
	std::vector<char> buffer(buffer_size(ent) * 2, fillchar);
	struct ::passwd pw;
	copy_to(ent, &pw, buffer.data());
	EXPECT_EQ(fillchar, buffer[buffer_size(ent)]);
	EXPECT_STREQ(ent.name().data(), pw.pw_name);
	EXPECT_STREQ(ent.password().data(), pw.pw_passwd);
	EXPECT_EQ(ent.id(), pw.pw_uid);
	EXPECT_EQ(ent.gid(), pw.pw_gid);
	EXPECT_STREQ(ent.shell().data(), pw.pw_shell);
	EXPECT_STREQ(ent.home().data(), pw.pw_dir);
	#ifdef __linux__
	EXPECT_STREQ(ent.real_name().data(), pw.pw_gecos);
	#endif
}
