#ifndef GGG_TEST_EXECUTE_COMMAND_HH
#define GGG_TEST_EXECUTE_COMMAND_HH

#include <string>

#include <unistdx/ipc/proc_status>

#include <gtest/gtest.h>


std::pair<sys::proc_status,std::string>
execute_command(const char* cmd);


#define ok(cmdline) EXPECT_EQ(0, ::system(cmdline))
#define fail(cmdline) EXPECT_NE(0, ::system(cmdline))
#define output_is(output, cmdline) \
	{ \
		auto result = ::execute_command(cmdline); \
		EXPECT_EQ(0, result.first.exit_code()); \
		EXPECT_EQ(output, result.second); \
	}


#endif // vim:filetype=cpp