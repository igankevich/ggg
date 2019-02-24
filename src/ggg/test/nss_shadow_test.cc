#include <ggg/core/days.hh>
#include <ggg/nss/entity_traits.hh>
#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>
#include <shadow.h>

struct sp_guard {
	sp_guard() { ::setspent(); }
	~sp_guard() { ::endspent(); }
};

TEST(shadow, empty) {
	{ Clean_database db; }
	sp_guard g;
	struct ::spwd* ent = ::getspent();
	ASSERT_EQ(nullptr, ent) << ent->sp_namp;
}

class shadow_test: public ::testing::TestWithParam<const char*> {};

TEST_P(shadow_test, CopyTo) {
	using namespace ggg;
	std::stringstream tmp;
	tmp << GetParam();
	ggg::account acc;
	tmp >> acc;
	char fillchar = -1;
	std::vector<char> buffer(buffer_size(acc) * 2, fillchar);
	struct ::spwd spw;
	std::memset(&spw, 0, sizeof(::spwd));
	copy_to(acc, &spw, buffer.data());
	EXPECT_EQ(fillchar, buffer[buffer_size(acc)]);
	EXPECT_STREQ(acc.login().data(), spw.sp_namp);
	EXPECT_STREQ("", spw.sp_pwdp);
	EXPECT_EQ(to_days(acc.last_change()), spw.sp_lstchg);
	EXPECT_EQ(to_days(acc.min_change()), spw.sp_min);
	EXPECT_EQ(to_days(acc.max_change()), spw.sp_max);
	EXPECT_EQ(to_days(acc.warn_change()), spw.sp_warn);
	EXPECT_EQ(to_days(acc.max_inactive()), spw.sp_inact);
	EXPECT_EQ(to_days(acc.expire()), spw.sp_expire);
}

INSTANTIATE_TEST_CASE_P(
	ReadWithWhiteSpace,
	shadow_test,
	::testing::Values(
		"bin:*:16605:0:99999:7:::",
		"bin : * : 16605 : 0 : 99999 : 7 :  :  : ",
		"bin:*:16605::99999:7:::",
		"bin:*:16605:0:99999:7:0:0:0"
	)
);

