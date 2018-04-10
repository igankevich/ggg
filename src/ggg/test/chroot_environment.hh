#ifndef GGG_TEST_CHROOT_ENVIRONMENT_HH
#define GGG_TEST_CHROOT_ENVIRONMENT_HH

#include <gtest/gtest.h>

class ChrootEnvironment: public ::testing::Environment {

public:

	void
	SetUp() override;

	void
	TearDown() override;

};

#endif // vim:filetype=cpp
