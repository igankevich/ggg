#include "entity.hh"
#include <gtest/gtest.h>

class EntityTest: public ::testing::TestWithParam<const char*> {};

TEST_P(EntityTest, Read) {
	std::stringstream tmp;
	tmp << GetParam();
	legion::entity ent;
	tmp >> ent;
	EXPECT_TRUE(tmp.good());
	EXPECT_STREQ("root", ent.name());
	EXPECT_STREQ("x", ent.password());
	EXPECT_EQ(12, ent.id());
	EXPECT_EQ(34, ent.group_id());
	#ifdef __linux__
	EXPECT_STREQ("root", ent.real_name());
	#else
	EXPECT_EQ(nullptr, ent.real_name());
	#endif
	EXPECT_STREQ("/root", ent.home());
	EXPECT_STREQ("/bin/bash", ent.shell());
}

INSTANTIATE_TEST_CASE_P(
	ReadWithWhiteSpace,
	EntityTest,
	::testing::Values(
		"root:x:12:34:root:/root:/bin/bash",
		"root:x: 12 : 34 :root:/root:/bin/bash",
		" root:x:12:34:root:/root:/bin/bash",
		"root :x:12:34:root:/root:/bin/bash",
		"root:x:12:34:root:/root:/bin/bash\n",
		" root : x : 12 : 34 : root : \t/root : /bin/bash\n"
	)
);

TEST(EntityTest, ReadWrite) {
	std::stringstream tmp, tmp2;
	tmp << "root:x:12:34:root:/root:/bin/bash";
	legion::entity ent, ent2;
	tmp >> ent;
	tmp2 << ent;
	EXPECT_EQ(tmp2.str(), tmp.str());
}

TEST(EntityTest, WriteEmpty) {
	std::stringstream orig;
	orig << "::" << sys::uid_type(-1) << ':' << sys::gid_type(-1) << ":::";
	std::stringstream tmp;
	legion::entity ent;
	tmp << ent;
	EXPECT_EQ(orig.str(), tmp.str());
}

class BareEntityTest: public ::testing::TestWithParam<const char*> {};

TEST_P(BareEntityTest, Read) {
	std::stringstream tmp;
	tmp << GetParam();
	legion::entity ent;
	tmp >> ent;
	EXPECT_TRUE(tmp.good());
	EXPECT_STREQ("root", ent.name());
	EXPECT_EQ(nullptr, ent.password());
	EXPECT_EQ(sys::uid_type(-1), ent.id());
	EXPECT_EQ(sys::gid_type(-1), ent.group_id());
	EXPECT_EQ(nullptr, ent.real_name());
	EXPECT_EQ(nullptr, ent.home());
	EXPECT_EQ(nullptr, ent.shell());
}

INSTANTIATE_TEST_CASE_P(
	ReadWithWhiteSpace,
	BareEntityTest,
	::testing::Values(
		"root\n",
		"root",
		" root",
		" root "
	)
);
