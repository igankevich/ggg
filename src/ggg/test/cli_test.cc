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
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | ggg add -f-)");
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u2" #:id 2002)' | ggg add -f-)");
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u3" #:id 2003)' | ggg add -f-)");
	EXPECT_NON_ZERO(R"(echo '(list (make <entity> #:name "u3" #:id 2003))' | ggg add -f-)")
	    << "added the same user twice";
	EXPECT_NON_ZERO(R"(echo '(list (make <entity> #:name "uu" #:id 2001))' | ggg add -f-)")
	    << "added a user with the same UID twice";
	EXPECT_NON_ZERO(R"(echo '(list (make <entity> #:name "u1" #:id 2222))' | ggg add -f-)")
	    << "added a user with the same USERNAME twice";
}

TEST_F(Commands, Remove) {
	EXPECT_OUTPUT("", "ggg find u1");
	EXPECT_ZERO("ggg rm u1");
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 3001)' | ggg add -f-)");
	EXPECT_OUTPUT("u1\n", "ggg find u1");
	EXPECT_ZERO("ggg rm u1");
	EXPECT_OUTPUT("", "ggg find u1");
	EXPECT_ZERO("ggg rm u1");
}

TEST_F(Commands, Info) {
	EXPECT_NON_ZERO("ggg info u1");
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | ggg add -f-)");
	EXPECT_ZERO("ggg info u1");
	EXPECT_ZERO("ggg rm u1");
	EXPECT_NON_ZERO("ggg info u1");
	EXPECT_NON_ZERO("ggg info donotexist");
}

//TEST_F(Commands, GetEnt) {
//	EXPECT_NON_ZERO("getent passwd u1");
//	EXPECT_NON_ZERO("getent group u1");
//	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -f-");
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
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | ggg add -f-)");
	EXPECT_OUTPUT("u1\n", "ggg find");
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u2" #:id 2002)' | ggg add -f-)");
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u3" #:id 2003)' | ggg add -f-)");
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
//	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | ggg add -f-");
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
//		tmp << "' | ggg add -f-";
//		std::clog << "tmp.str()=" << tmp.str() << std::endl;
//		EXPECT_ZERO(tmp.str().data());
//	}
//	EXPECT_ZERO("ggg duplicates");
//	auto result = execute_command("ggg duplicates");
//	EXPECT_EQ(0, result.first.exit_code());
//	EXPECT_TRUE(result.second.empty());
//}


TEST_F(Commands, GroupsAndMembers) {
	EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | ggg add -f-)");
	EXPECT_ZERO(R"(echo '(make <entity> #:name "g1" #:id 3001)' | ggg add -f-)");
	EXPECT_OUTPUT("", "ggg groups u1");
	EXPECT_OUTPUT("", "ggg members g1");
	EXPECT_ZERO("ggg tie u1 g1");
	EXPECT_OUTPUT("g1\n", "ggg groups u1");
	EXPECT_OUTPUT("u1\n", "ggg members g1");
	EXPECT_ZERO("ggg untie u1");
	EXPECT_OUTPUT("", "ggg groups u1");
	EXPECT_OUTPUT("", "ggg members g1");
	EXPECT_NON_ZERO("ggg groups donotexist");
	EXPECT_NON_ZERO("ggg members donotexist");
}

TEST_F(Commands, Version) {
	EXPECT_OUTPUT(GGG_VERSION "\n", "ggg version");
	EXPECT_OUTPUT(GGG_VERSION "\n", "ggg --version");
	EXPECT_OUTPUT(GGG_VERSION "\n", "ggg -v");
}

TEST_F(Commands, Edit) {
	EXPECT_ZERO(R"(ggg add -e '(make <entity> #:name "u1" #:id 2001 #:real-name "N" #:home-directory "H" #:shell "S")')");
	EXPECT_OUTPUT("u1::2001:2001:N:H:S\n", "ggg find -t u1");
	EXPECT_ZERO(R"(ggg edit -e '(make <entity> #:name "u1" #:id 2001 #:real-name "Nx" #:home-directory "H" #:shell "S")')");
	EXPECT_OUTPUT("u1::2001:2001:Nx:H:S\n", "ggg find -t u1");
	EXPECT_ZERO(R"(ggg edit -e '(make <entity> #:name "u1" #:id 2001 #:real-name "Nx" #:home-directory "Hx" #:shell "S")')");
	EXPECT_OUTPUT("u1::2001:2001:Nx:Hx:S\n", "ggg find -t u1");
	EXPECT_ZERO(R"(ggg edit -e '(make <entity> #:name "u1" #:id 2001 #:real-name "Nx" #:home-directory "Hx" #:shell "Sx")')");
	EXPECT_OUTPUT("u1::2001:2001:Nx:Hx:Sx\n", "ggg find -t u1");
	EXPECT_NON_ZERO(R"(ggg edit -e '(make <entity> #:name "u1" #:id 2222 #:real-name "Nx" #:home-directory "Hx" #:shell "Sx")')")
		<< "changed entity id but it is not allowed";
	EXPECT_OUTPUT("u1::2001:2001:Nx:Hx:Sx\n", "ggg find -t u1");
	EXPECT_ZERO(R"(ggg edit -e '(make <entity> #:name "u2" #:id 2001 #:real-name "Nx" #:home-directory "Hx" #:shell "Sx")')");
	EXPECT_OUTPUT("", "ggg find -t u1");
	EXPECT_OUTPUT("u2::2001:2001:Nx:Hx:Sx\n", "ggg find -t u2");
}

TEST_F(Commands, Locale) {
	EXPECT_ZERO(R"(ggg add -e '(make <entity> #:name "u1" #:id 2001 #:real-name "U1")')");
	EXPECT_ZERO(R"(ggg add -e '(make <entity> #:name "u2" #:id 2002 #:real-name "Ю2")')");
	EXPECT_ZERO(R"(ggg add -e '(make <entity> #:name "u3" #:id 2003 #:real-name "U3")')");
	EXPECT_ZERO("ggg lock u1 u2 u3");
	EXPECT_OUTPUT("u1\nu2\nu3\n", "env -i LC_ALL=C PATH=$PATH ggg locked");
}

TEST_F(Commands, TiesAndHierarchies) {
	EXPECT_ZERO(R"(ggg add -e '(make <entity> #:name "u1" #:id 2001 #:real-name "U1")')");
	EXPECT_ZERO(R"(ggg add -e '(make <entity> #:name "u2" #:id 2002 #:real-name "Ю2")')");
	EXPECT_ZERO(R"(ggg add -e '(make <entity> #:name "u3" #:id 2003 #:real-name "U3")')");
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
