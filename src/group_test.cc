#include "group.hh"
#include <gtest/gtest.h>
#include <vector>

TEST(EntityTest, CopyTo) {
	legion::group ent;
	char fillchar = -1;
	std::vector<char> buffer((ent.buffer_size() * 2) | 64, fillchar);
	struct ::group gr;
	ent.copy_to(&gr, buffer.data());
	EXPECT_EQ(fillchar, buffer[ent.buffer_size()]);
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

