#ifndef GGG_TEST_CHROOT_ENVIRONMENT_HH
#define GGG_TEST_CHROOT_ENVIRONMENT_HH

#include <unistd.h>

#include <gtest/gtest.h>

#include <iostream>

class ChrootEnvironment: public ::testing::Environment {

public:

	void
	SetUp() override;

	void
	TearDown() override;

};

inline void
skip_test_if_unpriviledged() {
	std::clog << "::getuid()=" << ::getuid() << std::endl;
	if (::getuid() != 0) {
		::_exit(77);
	}
}

#endif // vim:filetype=cpp
