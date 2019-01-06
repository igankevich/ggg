#include <sched.h>
#include <sys/mount.h>


#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include <unistdx/fs/file_status>
#include <unistdx/fs/path>
#include <unistdx/fs/canonical_path>

#include <ggg/config.hh>

#include <gtest/gtest.h>

#include <ggg/test/clean_database.hh>
#include <ggg/test/execute_command.hh>

class Commands: public ::testing::Test {

protected:

	void
	SetUp() override {
		EXPECT_ZERO("rm -rf " GGG_ROOT);
		EXPECT_ZERO("ggg init");
		Clean_database db;
	}

};

TEST(CommandsBase, Init) {
	EXPECT_ZERO("rm -rf " GGG_ROOT);
	EXPECT_ZERO("ggg init");
	sys::file_status root(GGG_ROOT);
	EXPECT_TRUE(root.exists());
	EXPECT_TRUE(root.is_directory());
	for (const auto* path : {GGG_ENTITIES_PATH, GGG_ACCOUNTS_PATH}) {
		sys::file_status db(path);
		EXPECT_TRUE(db.exists());
		EXPECT_TRUE(db.is_regular());
	}
}

TEST_F(Commands, Add) {
	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'u2:2002:U2:/home/u2:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -");
	EXPECT_NON_ZERO("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -")
	    << "added the same user twice";
	EXPECT_NON_ZERO("echo 'uu:2001:U1:/home/u1:/bin/sh' | ggg add -")
	    << "added a user with the same UID twice";
	EXPECT_NON_ZERO("echo 'u1:2222:U1:/home/u1:/bin/sh' | ggg add -")
	    << "added a user with the same USERNAME twice";
}

TEST_F(Commands, Remove) {
	EXPECT_OUTPUT("", "ggg find u1");
	EXPECT_ZERO("ggg rm u1");
	EXPECT_ZERO("echo 'u1:3001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_OUTPUT("u1\n", "ggg find u1");
	EXPECT_ZERO("ggg rm u1");
	EXPECT_OUTPUT("", "ggg find u1");
	EXPECT_ZERO("ggg rm u1");
}

TEST_F(Commands, Info) {
	EXPECT_NON_ZERO("ggg info u1");
	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_ZERO("ggg info u1");
	EXPECT_ZERO("ggg rm u1");
	EXPECT_NON_ZERO("ggg info u1");
	EXPECT_NON_ZERO("ggg info donotexist");
}

//TEST_F(Commands, GetEnt) {
//	EXPECT_NON_ZERO("getent passwd u1");
//	EXPECT_NON_ZERO("getent group u1");
//	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
//	EXPECT_OUTPUT("u1::2001:2001:U1:/home/u1:/bin/sh\n", "getent passwd u1");
//	EXPECT_OUTPUT("u1::2001:2001:U1:/home/u1:/bin/sh\n", "LANG=donotexist getent passwd u1");
//	EXPECT_OUTPUT("u1::2001:\n", "getent group u1");
//	EXPECT_OUTPUT("u1\n", "getent group u1 | cut -d: -f1");
//	EXPECT_OUTPUT("2001\n", "getent group u1 | cut -d: -f3");
//	EXPECT_ZERO("ggg rm u1");
//	EXPECT_NON_ZERO("getent passwd u1");
//	EXPECT_NON_ZERO("getent group u1");
//}

TEST_F(Commands, Find) {
	EXPECT_OUTPUT("", "ggg find");
	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_OUTPUT("u1\n", "ggg find");
	EXPECT_ZERO("echo 'u2:2002:U2:/home/u2:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -");
	EXPECT_OUTPUT("u1\n", "ggg find u1");
	EXPECT_OUTPUT("u1\n", "ggg find U1");
	EXPECT_OUTPUT("u2\n", "ggg find u2");
	EXPECT_OUTPUT("u3\n", "ggg find U3");
	EXPECT_OUTPUT("u1\nu2\nu3\n", "ggg find u | sort");
	EXPECT_OUTPUT("u1\nu2\n", "ggg find 'u[12]' | sort");
	EXPECT_OUTPUT("", "ggg find donotexist");
	EXPECT_ZERO("ggg rm u1");
	EXPECT_OUTPUT("", "ggg find u1");
}

//TEST_F(Commands, FindOutputMatchesGetEntOutput) {
//	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
//	auto result1 = execute_command("ggg find -t u1");
//	EXPECT_EQ(0, result1.first.exit_code());
//	auto result2 = execute_command("getent passwd u1");
//	EXPECT_EQ(0, result2.first.exit_code());
//	EXPECT_EQ(result2.second, result1.second);
//}
//
//TEST_F(Commands, Duplicates) {
//	auto uid = sys::this_process::user();
//	{
//		std::stringstream tmp;
//		tmp << "echo '";
//		tmp << "u1:" << uid << ":U1:/:/bin/sh";
//		tmp << "' | ggg add -";
//		std::clog << "tmp.str()=" << tmp.str() << std::endl;
//		EXPECT_ZERO(tmp.str().data());
//	}
//	EXPECT_ZERO("ggg duplicates");
//	auto result = execute_command("ggg duplicates");
//	EXPECT_EQ(0, result.first.exit_code());
//	EXPECT_TRUE(result.second.empty());
//}


TEST_F(Commands, GroupsAndMembers) {
	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'g1:3001:G1:/:/sbin/nologin' | ggg add -");
	EXPECT_OUTPUT("", "ggg groups u1");
	EXPECT_OUTPUT("", "ggg members g1");
//	EXPECT_ZERO("echo u1 > " GGG_ROOT "/ent/g1");
//	EXPECT_OUTPUT("g1\n", "ggg groups u1");
//	EXPECT_OUTPUT("u1\n", "ggg members g1");
//	EXPECT_ZERO("rm -f " GGG_ROOT "/ent/g1");
//	EXPECT_OUTPUT("", "ggg groups u1");
//	EXPECT_OUTPUT("", "ggg members g1");
	EXPECT_NON_ZERO("ggg groups donotexist");
	EXPECT_NON_ZERO("ggg members donotexist");
}

TEST_F(Commands, Version) {
	EXPECT_OUTPUT(GGG_VERSION "\n", "ggg version");
	EXPECT_OUTPUT(GGG_VERSION "\n", "ggg --version");
	EXPECT_OUTPUT(GGG_VERSION "\n", "ggg -v");
}

TEST_F(Commands, Edit) {
	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_OUTPUT("u1::2001:2001:U1:/home/u1:/bin/sh\n", "ggg find -t u1");
	EXPECT_ZERO("echo 'u1:2001:REAL NAME:/home/u1:/bin/sh' | ggg edit -");
	EXPECT_OUTPUT("u1::2001:2001:REAL NAME:/home/u1:/bin/sh\n", "ggg find -t u1");
	EXPECT_ZERO("echo 'u1:2001:REAL NAME:/home/HOME:/bin/sh' | ggg edit -");
	EXPECT_OUTPUT("u1::2001:2001:REAL NAME:/home/HOME:/bin/sh\n", "ggg find -t u1");
	EXPECT_ZERO("echo 'u1:2001:REAL NAME:/home/HOME:/bin/SHELL' | ggg edit -");
	EXPECT_OUTPUT(
		"u1::2001:2001:REAL NAME:/home/HOME:/bin/SHELL\n",
		"ggg find -t u1"
	);
	EXPECT_NON_ZERO("echo 'u1:2222:REAL NAME:/home/HOME:/bin/SHELL' | ggg edit -");
	EXPECT_OUTPUT(
		"u1::2001:2001:REAL NAME:/home/HOME:/bin/SHELL\n",
		"ggg find -t u1"
	);
	EXPECT_ZERO("echo 'u2:2001:REAL NAME:/home/HOME:/bin/SHELL' | ggg edit -");
	EXPECT_OUTPUT("", "ggg find -t u1");
	EXPECT_OUTPUT(
		"u2::2001:2001:REAL NAME:/home/HOME:/bin/SHELL\n",
		"ggg find -t u2"
	);
}

TEST_F(Commands, Form) {
	auto uid = sys::this_process::user();
	{
		std::stringstream tmp;
		tmp << "echo '";
		tmp << "u1:" << uid << ":U1:/:/bin/sh";
		tmp << "' | ggg add -";
		EXPECT_ZERO(tmp.str().data());
	}
	EXPECT_ZERO(
		R"(echo '
text:1:First name:^[A-Za-z ]+$
text:2:Last name:^[A-Za-zА-Яа-я ]+$
text:3:Username:^[A-Za-z0-9]+$
password:4:Password:^.{6,100}$
set:entropy:30.0
set:locale:ru_RU.UTF-8
set:entity.name:$3
set:entity.realname:$1 $2
set:entity.shell:/bin/bash
set:entity.homedir:/
set:entity.origin:u1/entities
set:entity.parent:u2
set:account.login:$3
set_secure:account.password:4
		')"
		"> " GGG_ROOT "/reg/u1"
	);
	EXPECT_ZERO("ggg init");
	EXPECT_ZERO("echo 'u2:2002:U2:/home/u2:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'F\nL\nu3\nnXq2UYKUD5yZd32jeN8M\nnXq2UYKUD5yZd32jeN8M\n' | ggg-form");
	EXPECT_OUTPUT("u3::2003:2003:F L:/:/bin/bash\n", "ggg find -t u3");
	EXPECT_OUTPUT("u2\n", "ggg parents u3");
}

TEST_F(Commands, EmptyForm) {
	auto uid = sys::this_process::user();
	{
		std::stringstream tmp;
		tmp << "echo '";
		tmp << "u1:" << uid << ":U1:/:/bin/sh";
		tmp << "' | ggg add -";
		EXPECT_ZERO(tmp.str().data());
	}
	EXPECT_ZERO("touch " GGG_ROOT "/reg/u1");
	EXPECT_ZERO("ggg init");
	EXPECT_NON_ZERO("echo 'F\nL\nu2\nnXq2UYKUD5yZd32jeN8M\n' | ggg-form");
}

//TEST_F(Commands, AddUnpriviledged) {
//	EXPECT_ZERO("echo 'u1:2001:U1:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("echo 'u2:2002:U2:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("touch " GGG_ROOT "/reg/u1");
//	EXPECT_ZERO("ggg init");
//	EXPECT_ZERO("su - u1 -c 'echo u3:2003:U3:/:/bin/sh | ggg add -'");
//	EXPECT_OUTPUT("u3::2003:2003:U3:/:/bin/sh\n", "ggg find -t u3");
//	EXPECT_OUTPUT("u3::2003:2003:U3:/:/bin/sh\n", "su - u1 -c 'ggg find -t u3'");
//	EXPECT_OUTPUT("u3::2003:2003:U3:/:/bin/sh\n", "su - u2 -c 'ggg find -t u3'");
//	EXPECT_OUTPUT("u3::2003:2003:U3:/:/bin/sh\n", "su - u3 -c 'ggg find -t u3'");
//	EXPECT_ZERO("ggg info u3");
//	EXPECT_ZERO("ggg info -a u3");
//}
//
//TEST_F(Commands, EditUnpriviledged) {
//	EXPECT_ZERO("echo 'u1:2001:U1:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("echo 'u2:2002:U2:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("echo 'u3:2003:U3:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("touch " GGG_ROOT "/reg/u1");
//	EXPECT_ZERO("touch " GGG_ROOT "/reg/u2");
//	EXPECT_ZERO("ggg init");
//	EXPECT_ZERO("su - u1 -c 'echo u4:2004:U4:/:/bin/sh | ggg add -'");
//	EXPECT_ZERO("su - u1 -c 'echo u4:2004:REALNAME:/:/bin/sh | ggg edit -'");
//	EXPECT_OUTPUT("u4::2004:2004:REALNAME:/:/bin/sh\n", "ggg find -t u4");
//	EXPECT_OUTPUT(
//		"u4::2004:2004:REALNAME:/:/bin/sh\n",
//		"su - u1 -c 'ggg find -t u4'"
//	);
//	EXPECT_OUTPUT(
//		"u4::2004:2004:REALNAME:/:/bin/sh\n",
//		"su - u2 -c 'ggg find -t u4'"
//	);
//	EXPECT_OUTPUT(
//		"u4::2004:2004:REALNAME:/:/bin/sh\n",
//		"su - u3 -c 'ggg find -t u4'"
//	);
//	EXPECT_NON_ZERO("su - u2 -c 'echo u4:2004:NEWNAME:/:/bin/sh | ggg edit -'");
//	EXPECT_NON_ZERO("su - u3 -c 'echo u4:2004:NEWNAME:/:/bin/sh | ggg edit -'");
//	EXPECT_NON_ZERO("su - u4 -c 'echo u4:2004:NEWNAME:/:/bin/sh | ggg edit -'");
//}
//
//TEST_F(Commands, RemoveUnpriviledged) {
//	EXPECT_ZERO("echo 'u1:2001:U1:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("echo 'u2:2002:U2:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("touch " GGG_ROOT "/reg/u1");
//	EXPECT_ZERO("ggg init");
//	EXPECT_OUTPUT("", "ggg find u3");
//	EXPECT_ZERO("su - u1 -c 'echo u3:2003:U3:/:/bin/sh | ggg add -'");
//	EXPECT_OUTPUT("u3\n", "ggg find u3");
//	EXPECT_ZERO("su - u1 -c 'ggg rm u3'");
//	EXPECT_OUTPUT("", "ggg find u3");
//}
//
//TEST_F(Commands, Cache) {
//	EXPECT_ZERO("mkdir -p /var/cache/ggg");
//	EXPECT_ZERO("echo 'u1:2001:U1:/:/bin/sh' | ggg add -");
//	EXPECT_ZERO("getent passwd u1");
//	EXPECT_ZERO("ls " GGG_CACHE_DIRECTORY "/ggg/*");
//	ASSERT_EQ(0, ::system("rm -rf " GGG_ENT_ROOT "/*"));
//	EXPECT_ZERO("getent passwd u1");
//	EXPECT_ZERO("rm -rf /var/cache/ggg");
//	EXPECT_NON_ZERO("getent passwd u1");
//}

TEST_F(Commands, Locale) {
	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'u2:2002:Ю2:/home/u2:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -");
	EXPECT_ZERO("ggg lock u1 u2 u3");
	EXPECT_OUTPUT("u1\nu2\nu3\n", "env -i LC_ALL=C PATH=$PATH ggg locked");
}

TEST_F(Commands, TiesAndHierarchies) {
	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'u2:2002:Ю2:/home/u2:/bin/sh' | ggg add -");
	EXPECT_ZERO("echo 'u3:2003:U3:/home/u3:/bin/sh' | ggg add -");
	EXPECT_ZERO("ggg attach u1 u2");
	EXPECT_NON_ZERO("ggg attach u1 u2")
		<< "attached same entities multiple times";
	EXPECT_NON_ZERO("ggg tie u1 u2")
		<< "tied entitites with the same hierarhy root";
	EXPECT_ZERO("ggg detach u1");
	EXPECT_ZERO("ggg tie u1 u2");
	EXPECT_NON_ZERO("ggg attach u1 u2") << "attached tied entities";
	EXPECT_ZERO("ggg untie u1 u2");
	EXPECT_ZERO("ggg attach u1 u2");
}

int
main(int argc, char* argv[]) {
	const char* ggg_executable = getenv("GGG_EXECUTABLE");
	if (!ggg_executable) {
		return 77;
	}
	sys::canonical_path ggg_exe(ggg_executable);
	if (const char* path = getenv("PATH")) {
		std::string new_path = ggg_exe.dirname();
		new_path += ':';
		new_path += path;
		setenv("PATH", new_path.data(), 1);
	} else {
		setenv("PATH", ggg_exe.dirname(), 1);
	}
//	skip_test_if_unpriviledged();
	::testing::InitGoogleTest(&argc, argv);
//	::testing::AddGlobalTestEnvironment(new ChrootEnvironment);
	return RUN_ALL_TESTS();
}
