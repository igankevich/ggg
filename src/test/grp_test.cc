#include <gtest/gtest.h>
#include "config.hh"
#include <grp.h>
#include "nss/gr_guard.hh"
#include "string_replace.hh"

void
init_root(const char* script) {
	ASSERT_EQ(0, run_script(script, GGG_ROOT));
}

class GrpTest: public ::testing::Test {};

class GrpParamTest:
public GrpTest,
public ::testing::WithParamInterface<const char*>
{};

TEST(Grp, Empty) {
	init_root(R"(
	rm -f passwd group
	touch passwd group
	rm -rf %
	mkdir %
	)");
	gr_guard g;
	struct ::group* ent = ::getgrent();
	ASSERT_EQ(nullptr, ent) << ent->gr_name;
}

TEST(Grp, GroupList) {
	init_root(R"(
	rm -f passwd group
	touch passwd group
	rm -rf %
	mkdir %
	echo 'testuser:x:2000:2000:halt:/sbin:/sbin/halt' > %/file
	)");
	int ngroups = 4096 / sizeof(::gid_t);
	::gid_t groups[ngroups];
	int ret = ::getgrouplist("testuser", 2000, groups, &ngroups);
	ASSERT_EQ(1, ret);
}
