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
	EXPECT_STREQ(nullptr, ent.real_name());
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
	std::stringstream tmp;
	legion::entity ent;
	tmp << ent;
	EXPECT_EQ("::0:0:::", tmp.str());
}
