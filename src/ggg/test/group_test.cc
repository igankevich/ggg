#include <vector>

#include <ggg/core/group.hh>

#include <gtest/gtest.h>

class GroupTest: public ::testing::TestWithParam<ggg::group> {};

TEST(GroupTest2, CopyTo) {
	ggg::group ent;
	char fillchar = -1;
	std::vector<char> buffer((ggg::buffer_size(ent) * 2) | 64, fillchar);
	struct ::group gr;
	ggg::copy_to(ent, &gr, buffer.data());
	EXPECT_EQ(fillchar, buffer[ggg::buffer_size(ent)]);
	EXPECT_STREQ(ent.name().data(), gr.gr_name);
	EXPECT_EQ(ent.id(), gr.gr_gid);
	EXPECT_STREQ(ent.password().data(), gr.gr_passwd);
	std::unordered_set<std::string> mem;
	char** first = gr.gr_mem;
	while (*first) {
		mem.emplace(*first);
		++first;
	}
	EXPECT_EQ(ent.members(), mem);
}


TEST_P(GroupTest, BstreamIO) {
	ggg::group ent(GetParam()), ent2;
	std::stringbuf buf;
	sys::bstream str(&buf);
	str << ent;
	str >> ent2;
	EXPECT_EQ(ent, ent2);
}

INSTANTIATE_TEST_CASE_P(
	All,
	GroupTest,
	::testing::Values(
		ggg::group(),
		ggg::group("g1"),
		ggg::group("g1", "x", 1231),
		ggg::group("g1", "x", 1231, {"a", "b", "c"}),
		ggg::group("g1", "x", 1231, {"a", "b"}),
		ggg::group("g1", "x", 1231, {"a"})
	)
);

