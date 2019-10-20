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

TEST(Account2, GetIdAndSalt) {
	std::stringstream tmp;
	tmp << "dummy:$6$hMYBE0GG$8itF7ypR3LKiGDObpuVK.4T2.y6Y0GEVZXbvrguwI933HADWIfG999USIBvPwZto18yv5Fp5o46GG9JvUO9sU.:::::::";
	ggg::account acc;
	tmp >> acc;
	std::clog << "acc=" << acc << std::endl;
	EXPECT_EQ("6", acc.password_id());
	EXPECT_EQ("hMYBE0GG", acc.password_salt());
	EXPECT_EQ("$6$hMYBE0GG$", acc.password_prefix());
}

TEST(Account2, GetRounds) {
	std::stringstream tmp;
	tmp << "dummy:$6$rounds=1000$hMYBE0GG$8itF7ypR3LKiGDObpuVK.4T2.y6Y0GEVZXbvrguwI933HADWIfG999USIBvPwZto18yv5Fp5o46GG9JvUO9sU.:::::::";
	ggg::account acc;
	tmp >> acc;
	std::clog << "acc=" << acc << std::endl;
	EXPECT_EQ("6", acc.password_id());
	EXPECT_EQ("hMYBE0GG", acc.password_salt());
	EXPECT_EQ("$6$rounds=1000$hMYBE0GG$", acc.password_prefix());
	EXPECT_EQ(1000u, acc.num_rounds());
}

TEST(Account2, ExpireActive) {
	ggg::account acc;
	acc.make_expired();
	EXPECT_TRUE(acc.has_expired());
	acc.make_active();
	EXPECT_FALSE(acc.has_expired());
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

