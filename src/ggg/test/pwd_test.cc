#include <gtest/gtest.h>

#include <pwd.h>

#include <ggg/config.hh>
#include <ggg/nss/pw_guard.hh>

#include "string_replace.hh"

void
init_root(const char* script) {
	ASSERT_EQ(0, run_script(script, GGG_ENT_ROOT));
}

class PwdTest: public ::testing::Test {};

class PwdParamTest:
public PwdTest,
public ::testing::WithParamInterface<const char*>
{};

TEST(Pwd, Empty) {
	init_root(R"(
	rm -f passwd group
	touch passwd group
	rm -rf %
	mkdir -p %
	)");
	pw_guard g;
	struct ::passwd* ent = ::getpwent();
	ASSERT_EQ(nullptr, ent) << ent->pw_name;
}

TEST(Pwd, SingleEntry) {
	init_root(R"(
	rm -f passwd group
	touch passwd group
	rm -rf %
	mkdir -p %
	echo 'testuser:x:2000:2000:halt:/sbin:/sbin/halt' > %/file
	)");
	pw_guard g;
	struct ::passwd* ent = ::getpwent();
	ASSERT_NE(nullptr, ent);
	EXPECT_STREQ("testuser", ent->pw_name);
}

TEST(Pwd, GetByName) {
	init_root(R"(
	rm -f passwd group
	touch passwd group
	rm -rf %
	mkdir -p %
	echo 'testuser:x:2000:2000:halt:/sbin:/sbin/halt' >> %/file
	echo 'user2:x:2001:2001:halt:/sbin:/sbin/halt' >> %/file
	echo 'user3:x:2002:2002:halt:/sbin:/sbin/halt' >> %/file
	)");
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

TEST(Pwd, GetByUid) {
	init_root(R"(
	rm -f passwd group
	touch passwd group
	rm -rf %
	mkdir -p %
	echo 'testuser:x:2000:2000:halt:/sbin:/sbin/halt' >> %/file
	echo 'user2:x:2001:2001:halt:/sbin:/sbin/halt' >> %/file
	echo 'user3:x:2002:2002:halt:/sbin:/sbin/halt' >> %/file
	)");
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
