#ifndef GGG_TEST_EXECUTE_COMMAND_HH
#define GGG_TEST_EXECUTE_COMMAND_HH

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>

#include <unistdx/ipc/process_status>

#include <gtest/gtest.h>


std::tuple<sys::process_status,std::string,std::string>
execute_command(const char* cmd);

#define EXPECT_ZERO2(cmdline, message) \
	{ \
		auto result = ::execute_command(cmdline); \
        auto exit_code =  std::get<0>(result).exit_code(); \
        if (exit_code != 0) { \
            std::cout << std::get<1>(result) << std::flush; \
            std::clog << std::get<2>(result) << std::flush; \
        } \
        EXPECT_EQ(0, exit_code) << message; \
	}

#define EXPECT_ZERO(cmdline) \
    EXPECT_ZERO2(cmdline, "")

#define EXPECT_NON_ZERO2(cmdline, message) \
	{ \
		auto result = ::execute_command(cmdline); \
        auto exit_code =  std::get<0>(result).exit_code(); \
        if (exit_code == 0) { \
            std::cout << std::get<1>(result) << std::flush; \
            std::clog << std::get<2>(result) << std::flush; \
        } \
        EXPECT_NE(0, exit_code) << message; \
	}

#define EXPECT_NON_ZERO(cmdline) \
    EXPECT_NON_ZERO2(cmdline, "")

#define EXPECT_OUTPUT(output, cmdline) \
	{ \
		auto result = ::execute_command(cmdline); \
        auto actual = std::get<1>(result); \
        if (output != actual || std::get<0>(result).exit_code() != 0) { \
            std::clog << std::get<2>(result) << std::flush; \
        } \
		EXPECT_EQ(0, std::get<0>(result).exit_code()); \
		EXPECT_EQ(output, actual); \
	}

#endif // vim:filetype=cpp
