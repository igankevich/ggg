#include <ggg/core/entity.hh>
#include <ggg/core/group.hh>
#include <gtest/gtest.h>
#include <vector>

class EntityTest: public ::testing::TestWithParam<const char*> {};

TEST_P(EntityTest, Read) {
	std::stringstream tmp;
	tmp << GetParam();
	ggg::entity ent;
	tmp >> ent;
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ("root", ent.name());
	EXPECT_EQ("x", ent.password());
	EXPECT_EQ(12, ent.id());
	EXPECT_EQ(34, ent.gid());
	#ifdef __linux__
	EXPECT_EQ("root", ent.real_name());
	#else
	EXPECT_EQ("", ent.real_name());
	#endif
	EXPECT_EQ("/root", ent.home());
	EXPECT_EQ("/bin/bash", ent.shell());
}

TEST_P(EntityTest, Clear) {
	std::stringstream tmp;
	tmp << GetParam();
	ggg::entity ent;
	tmp >> ent;
	ent.clear();
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ("", ent.name());
	EXPECT_EQ("", ent.password());
	EXPECT_EQ(sys::uid_type(-1), ent.id());
	EXPECT_EQ(sys::gid_type(-1), ent.gid());
	#ifdef __linux__
	EXPECT_EQ("", ent.real_name());
	#else
	EXPECT_EQ("", ent.real_name());
	#endif
	EXPECT_EQ("", ent.home());
	EXPECT_EQ("", ent.shell());
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
	ggg::entity ent, ent2;
	tmp >> ent;
	tmp2 << ent;
	EXPECT_EQ(tmp2.str(), tmp.str());
}

TEST(EntityTest, WriteEmpty) {
	std::stringstream orig;
	orig << "::" << sys::uid_type(-1) << ':' << sys::gid_type(-1) << ":::";
	std::stringstream tmp;
	ggg::entity ent;
	tmp << ent;
	EXPECT_EQ(orig.str(), tmp.str());
}

class BareEntityTest: public ::testing::TestWithParam<const char*> {};

TEST_P(BareEntityTest, Read) {
	std::stringstream tmp;
	tmp << GetParam();
	ggg::entity ent;
	tmp >> ent;
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ("root", ent.name());
	EXPECT_EQ("", ent.password());
	EXPECT_EQ(sys::uid_type(-1), ent.id());
	EXPECT_EQ(sys::gid_type(-1), ent.gid());
	EXPECT_EQ("", ent.real_name());
	EXPECT_EQ("", ent.home());
	EXPECT_EQ("", ent.shell());
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

TEST(EntityTest, ReadEntryWithMissingFields) {
	std::stringstream tmp;
	tmp << "mygroup:x:2000:2000:mygroup name:/home:\n";
	ggg::entity ent;
	tmp >> ent;
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ("mygroup", ent.name());
	EXPECT_EQ("x", ent.password());
	EXPECT_EQ(2000, ent.id());
	EXPECT_EQ(2000, ent.gid());
	EXPECT_EQ("/home", ent.home());
	EXPECT_EQ("", ent.shell());
}

TEST(EntityTest, CopyTo) {
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

TEST_P(EntityTest, BstreamIO) {
	ggg::entity ent, ent2;
	{
		std::stringstream tmp;
		tmp << GetParam();
		tmp >> ent;
	}
	std::stringbuf buf;
	sys::bstream str(&buf);
	str << ent;
	str >> ent2;
	EXPECT_EQ(ent, ent2);
}
