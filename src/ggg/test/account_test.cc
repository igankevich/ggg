#include <gtest/gtest.h>
#include <sstream>
#include <chrono>
#include <vector>
#include <cstring>
#include <ggg/core/account.hh>

class AccountTest: public ::testing::TestWithParam<const char*> {};

class Account2Test: public ::testing::Test {};

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

TEST(Account2, ExpireActive) {
	ggg::account acc;
	acc.make_expired();
    auto now = ggg::account::clock_type::now();
	EXPECT_TRUE(acc.has_expired(now));
	acc.make_active();
    now = ggg::account::clock_type::now();
	EXPECT_FALSE(acc.has_expired(now));
}

TEST(Account2, ReadDefaultConstructed) {
	ggg::account acc0;
	std::stringstream tmp;
	tmp << acc0;
	ggg::account acc;
	tmp >> acc;
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ("", acc.login());
	EXPECT_EQ("", acc.password());
	EXPECT_EQ(0L, to_days(acc.last_change()));
	EXPECT_EQ(0L, to_days(acc.min_change()));
	EXPECT_EQ(0L, to_days(acc.max_change()));
	EXPECT_EQ(0L, to_days(acc.warn_change()));
	EXPECT_EQ(0L, to_days(acc.max_inactive()));
	EXPECT_EQ(0L, to_days(acc.expire()));
}

TEST_P(AccountTest, Clear) {
	std::stringstream tmp;
	tmp << GetParam();
	ggg::account acc;
	tmp >> acc;
	acc.clear();
	EXPECT_TRUE(tmp.good());
	EXPECT_EQ("", acc.login());
	EXPECT_EQ("", acc.password());
	EXPECT_EQ(0L, to_days(acc.last_change()));
	EXPECT_EQ(0L, to_days(acc.min_change()));
	EXPECT_EQ(0L, to_days(acc.max_change()));
	EXPECT_EQ(0L, to_days(acc.warn_change()));
	EXPECT_EQ(0L, to_days(acc.max_inactive()));
	EXPECT_EQ(0L, to_days(acc.expire()));
}

