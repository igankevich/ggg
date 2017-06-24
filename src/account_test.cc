#include <gtest/gtest.h>
#include <sstream>
#include <chrono>
#include <vector>
#include "account.hh"

class AccountTest: public ::testing::TestWithParam<const char*> {};

long
to_days(ggg::account::time_point tp) {
	using namespace std::chrono;
	return time_point_cast<hours>(tp).time_since_epoch().count() / 24L;
}

long
to_days(ggg::account::duration d) {
	using namespace std::chrono;
	return duration_cast<hours>(d).count() / 24L;
}

TEST_P(AccountTest, Read) {
	std::stringstream tmp;
	tmp << GetParam();
	ggg::account acc;
	tmp >> acc;
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ("bin", acc.login());
	EXPECT_EQ("*", acc.password());
	EXPECT_EQ(16605L, to_days(acc.last_change()));
	EXPECT_EQ(0L, to_days(acc.min_change()));
	EXPECT_EQ(99999L, to_days(acc.max_change()));
	EXPECT_EQ(7L, to_days(acc.warn_change()));
	EXPECT_EQ(0L, to_days(acc.max_inactive()));
	EXPECT_EQ(0L, to_days(acc.expire()));
}

TEST_P(AccountTest, ReadWrite) {
	std::stringstream tmp, tmp2;
	tmp << "bin:*:16605::99999:7:::";
	ggg::account acc;
	tmp >> acc;
	tmp2 << acc;
	EXPECT_EQ(tmp2.str(), tmp.str());
}

TEST_P(AccountTest, CopyTo) {
	std::stringstream tmp;
	tmp << GetParam();
	ggg::account acc;
	tmp >> acc;
	char fillchar = -1;
	std::vector<char> buffer(acc.buffer_size() * 2, fillchar);
	struct ::spwd spw;
	acc.copy_to(&spw, buffer.data());
	EXPECT_EQ(fillchar, buffer[acc.buffer_size()]);
	EXPECT_STREQ(acc.login().data(), spw.sp_namp);
	EXPECT_STREQ(acc.password().data(), spw.sp_pwdp);
	EXPECT_EQ(to_days(acc.last_change()), spw.sp_lstchg);
	EXPECT_EQ(to_days(acc.min_change()), spw.sp_min);
	EXPECT_EQ(to_days(acc.max_change()), spw.sp_max);
	EXPECT_EQ(to_days(acc.warn_change()), spw.sp_warn);
	EXPECT_EQ(to_days(acc.max_inactive()), spw.sp_inact);
	EXPECT_EQ(to_days(acc.expire()), spw.sp_expire);
}

INSTANTIATE_TEST_CASE_P(
	ReadWithWhiteSpace,
	AccountTest,
	::testing::Values(
		"bin:*:16605:0:99999:7:::",
		"bin : * : 16605 : 0 : 99999 : 7 :  :  : ",
		"bin:*:16605::99999:7:::",
		"bin:*:16605:0:99999:7:0:0:0"
	)
);
