#include <gtest/gtest.h>
#include "config.hh"
#include <pwd.h>

TEST(pw, setpwent) {
	std::clog << "HIERARCHY_ROOT=" << HIERARCHY_ROOT << std::endl;
	::setpwent();
	::endpwent();
}
