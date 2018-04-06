#include <sched.h>
#include <sys/mount.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <unistdx/base/check>
#include <unistdx/fs/canonical_path>
#include <unistdx/fs/idirtree>
#include <unistdx/fs/path>
#include <unistdx/fs/pathentry>
#include <unistdx/io/pipe>
#include <unistdx/io/fdstream>
#include <unistdx/io/open_flag>
#include <unistdx/ipc/execute>
#include <unistdx/ipc/process>

#include <ggg/config.hh>

#include <gtest/gtest.h>

class ChrootEnvironment: public ::testing::Environment {

public:

	void
	SetUp() override {
//		UNISTDX_CHECK(::unshare(CLONE_NEWNS));
		sys::canonical_path overlay(std::getenv("GGG_OVERLAY"));
		sys::path myroot(overlay, "myroot");
		sys::path newroot(overlay, "newroot");
		sys::path workdir(overlay, "workdir");
		std::stringstream opts;
		opts << "lowerdir=/,upperdir=" << myroot << ",workdir=" << workdir;
		UNISTDX_CHECK(
			::mount(
				"overlay",
				newroot,
				"overlay",
				0,
				opts.str().data()
			)
		);
		UNISTDX_CHECK(::chroot(newroot));
		UNISTDX_CHECK(::mount("devtmpfs", "/dev", "devtmpfs", 0, nullptr));
		UNISTDX_CHECK(::mount("devpts", "/dev/pts", "devpts", 0, nullptr));
		UNISTDX_CHECK(::mount("proc", "/proc", "proc", 0, nullptr));
	}

};


class Commands: public ::testing::Test {

protected:

	void
	SetUp() override {
		ASSERT_EQ(0, ::system("rm -rf " GGG_ROOT));
		ASSERT_EQ(0, ::system("ggg init"));
	}

};

namespace {

	std::pair<sys::proc_status,std::string>
	execute_command(const char* cmd) {
		sys::pipe pipe;
		pipe.in().unsetf(sys::open_flag::non_blocking);
		pipe.out().unsetf(sys::open_flag::non_blocking);
		sys::process child{
			[cmd,&pipe] () {
				pipe.in().close();
				pipe.out().remap(STDOUT_FILENO);
				return sys::this_process::execute_command("/bin/sh", "-c", cmd);
			}
		};
		pipe.out().close();
		sys::ifdstream in(std::move(pipe.in()));
		std::stringstream tmp;
		tmp << in.rdbuf();
		return std::make_pair(child.wait(), tmp.str());
	}

}

#define ok(cmdline) EXPECT_EQ(0, ::system(cmdline))
#define fail(cmdline) EXPECT_NE(0, ::system(cmdline))
#define output_is(output, cmdline) \
	{ \
		auto result = ::execute_command(cmdline); \
		EXPECT_EQ(0, result.first.exit_code()); \
		EXPECT_EQ(output, result.second); \
	}

TEST(CommandsBase, Init) {
	ok("ggg init");
	sys::file_stat root(GGG_ROOT);
	EXPECT_TRUE(root.exists());
	EXPECT_TRUE(root.is_directory());
}

TEST_F(Commands, Add) {
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	ok("echo 'u2:2002:U2:/home/u2:/bin/sh' | ggg add -");
	ok("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -");
	fail("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -")
	    << "added the same user twice";
	fail("echo 'uu:2001:U1:/home/u1:/bin/sh' | ggg add -")
	    << "added a user with the same UID twice";
	fail("echo 'u1:2222:U1:/home/u1:/bin/sh' | ggg add -")
	    << "added a user with the same USERNAME twice";
}

TEST_F(Commands, Remove) {
	ok("ggg rm u1");
	ok("echo 'u1:3001:U1:/home/u1:/bin/sh' | ggg add -");
	ok("ggg rm u1");
	ok("ggg rm u1");
}

TEST_F(Commands, Info) {
	fail("ggg info u1");
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	ok("ggg info u1");
	ok("ggg rm u1");
	fail("ggg info u1");
	fail("ggg info donotexist");
}

TEST_F(Commands, GetEnt) {
	fail("getent passwd u1");
	fail("getent group u1");
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	output_is("u1::2001:2001:U1:/home/u1:/bin/sh\n", "getent passwd u1");
	output_is("u1::2001:\n", "getent group u1");
	output_is("u1\n", "getent group u1 | cut -d: -f1");
	output_is("2001\n", "getent group u1 | cut -d: -f3");
	ok("ggg rm u1");
	fail("getent passwd u1");
	fail("getent group u1");
}

TEST_F(Commands, Find) {
	output_is("", "ggg find");
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	output_is("u1\n", "ggg find");
	ok("echo 'u2:2002:U2:/home/u2:/bin/sh' | ggg add -");
	ok("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -");
	output_is("u1\n", "ggg find u1");
	output_is("u1\n", "ggg find U1");
	output_is("u2\n", "ggg find u2");
	output_is("u3\n", "ggg find U3");
	output_is("u1\nu2\nu3\n", "ggg find u | sort");
	output_is("u1\nu2\n", "ggg find 'u[12]' | sort");
	output_is("", "ggg find donotexist");
	ok("ggg rm u1");
	output_is("", "ggg find u1");
}

TEST_F(Commands, FindOutputMatchesGetEntOutput) {
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	auto result1 = execute_command("ggg find -t u1");
	EXPECT_EQ(0, result1.first.exit_code());
	auto result2 = execute_command("getent passwd u1");
	EXPECT_EQ(0, result2.first.exit_code());
	EXPECT_EQ(result2.second, result1.second);
}

TEST_F(Commands, Duplicates) {
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	ok("ggg duplicates");
	ok("echo 'u2::2001:2001:U2:/home/u2:/bin/sh' > " GGG_ROOT "/ent/dup");
	auto result = execute_command("ggg duplicates");
	EXPECT_NE(0, result.first.exit_code());
	EXPECT_FALSE(result.second.empty());
}

TEST_F(Commands, GroupsAndMembers) {
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	ok("echo 'g1:3001:G1:/:/sbin/nologin' | ggg add -");
	output_is("", "ggg groups u1");
	output_is("", "ggg members g1");
	ok("echo u1 > " GGG_ROOT "/ent/g1");
	output_is("g1\n", "ggg groups u1");
	output_is("u1\n", "ggg members g1");
	ok("rm -f " GGG_ROOT "/ent/g1");
	output_is("", "ggg groups u1");
	output_is("", "ggg members g1");
}

TEST_F(Commands, Version) {
	output_is(GGG_VERSION "\n", "ggg version");
	output_is(GGG_VERSION "\n", "ggg --version");
	output_is(GGG_VERSION "\n", "ggg -v");
}

TEST_F(Commands, Edit) {
	ok("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	output_is("u1::2001:2001:U1:/home/u1:/bin/sh\n", "ggg find -t u1");
	ok("echo 'u1:2001:REAL NAME:/home/u1:/bin/sh' | ggg edit -");
	output_is("u1::2001:2001:REAL NAME:/home/u1:/bin/sh\n", "ggg find -t u1");
	ok("echo 'u1:2001:REAL NAME:/home/HOME:/bin/sh' | ggg edit -");
	output_is("u1::2001:2001:REAL NAME:/home/HOME:/bin/sh\n", "ggg find -t u1");
	ok("echo 'u1:2001:REAL NAME:/home/HOME:/bin/SHELL' | ggg edit -");
	output_is("u1::2001:2001:REAL NAME:/home/HOME:/bin/SHELL\n", "ggg find -t u1");
	ok("echo 'u1:2222:REAL NAME:/home/HOME:/bin/SHELL' | ggg edit -");
	output_is("u1::2222:2222:REAL NAME:/home/HOME:/bin/SHELL\n", "ggg find -t u1");
	ok("echo 'u2:2222:REAL NAME:/home/HOME:/bin/SHELL' | ggg edit -");
	output_is("", "ggg find -t u1");
	output_is("u2::2222:2222:REAL NAME:/home/HOME:/bin/SHELL\n", "ggg find -t u2");
}

TEST_F(Commands, Form) {
	ok("echo 'u1:2001:U1:/:/bin/sh' | ggg add -");
	ok(
		R"(echo '
text:1:First name:^[A-Za-z ]+$
text:2:Last name:^[A-Za-zА-Яа-я ]+$
text:3:Username:^[A-Za-z0-9]+$
password:4:Password:^.{6,100}$
set:entropy:30.0
set:entity.name:$3
set:entity.realname:$1 $2
set:entity.shell:/bin/bash
set:entity.homedir:/
set:entity.origin:u1/entities
set:account.login:$3
set_secure:account.password:4
		')"
		"> " GGG_ROOT "/reg/u1"
	);
	ok("ggg init");
	ok("echo 'F\nL\nu2\nnXq2UYKUD5yZd32jeN8M\n' | su - u1 -c ggg-form");
	output_is("u2::2002:2002:F L:/:/bin/bash\n", "ggg find -t u2");
	output_is("u2::2002:2002:F L:/:/bin/bash\n", "getent passwd u2");
	ok("su - u1 -c 'echo u3:2003:U3:/:/bin/sh | ggg add -'");
	output_is("u3::2003:2003:U3:/:/bin/sh\n", "ggg find -t u3");
	output_is("u3::2003:2003:U3:/:/bin/sh\n", "su - u1 -c 'ggg find -t u3'");
	output_is("u3::2003:2003:U3:/:/bin/sh\n", "su - u2 -c 'ggg find -t u3'");
	output_is("u3::2003:2003:U3:/:/bin/sh\n", "su - u3 -c 'ggg find -t u3'");
}

int
main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new ChrootEnvironment);
	return RUN_ALL_TESTS();
}
