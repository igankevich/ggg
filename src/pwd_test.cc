#include <gtest/gtest.h>
#include "config.hh"
#include <pwd.h>
#include "pw_guard.hh"
#include "string_replace.hh"

void
init_root(const char* script) {
	std::clog << "script=" << script << std::endl;
	ASSERT_EQ(0, run_script(script, HIERARCHY_ROOT));
}

class PwdTest: public ::testing::Test {};

class PwdParamTest:
public PwdTest,
public ::testing::WithParamInterface<const char*>
{};

TEST(pw, setpwent) {
	std::clog << "HIERARCHY_ROOT=" << HIERARCHY_ROOT << std::endl;
	init_root(R"(
	rm -f passwd group
	touch passwd group
	rm -rf %
	mkdir %
	)");
	pw_guard g;
	struct ::passwd* ent = ::getpwent();
	ASSERT_EQ(nullptr, ent) << ent->pw_name;
}
