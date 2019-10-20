#include <gtest/gtest.h>

#include <ggg/pam/conversation.hh>

TEST(Messages, Invariants) {
	ggg::messages m;
	EXPECT_EQ(0u, m.size());
	EXPECT_EQ(0, m.end() - m.begin());
	EXPECT_EQ(nullptr, (const pam_message**)m);
	m.emplace_back(PAM_TEXT_INFO, "hello");
	EXPECT_EQ(1u, m.size());
	EXPECT_EQ(1, m.end() - m.begin());
	EXPECT_NE(nullptr, (const pam_message**)m);
}

TEST(Response, Invariants) {
	ggg::responses r(1);
	EXPECT_EQ(1u, r.size());
	EXPECT_EQ(1, r.end() - r.begin());
	EXPECT_NE(nullptr, (pam_response**)r);
	EXPECT_EQ(nullptr, *((pam_response**)r));
	EXPECT_FALSE(r.ok());
}
