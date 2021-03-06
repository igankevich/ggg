#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include <unistdx/fs/file_status>
#include <unistdx/fs/path>
#include <unistdx/fs/canonical_path>

#include <gtest/gtest.h>

#include <ggg/config.hh>
#include <ggg/proto/daemon_environment.hh>
#include <ggg/test/clean_database.hh>
#include <ggg/test/execute_command.hh>

class Commands: public ::testing::Test {

protected:

    void
    SetUp() override {
        /*
        int ret;
        ret = std::remove(GGG_ENTITIES_PATH);
        EXPECT_TRUE(ret == 0 || (ret == -1 && errno == ENOENT));
        ret = std::remove(GGG_ACCOUNTS_PATH);
        EXPECT_TRUE(ret == 0 || (ret == -1 && errno == ENOENT));
        EXPECT_ZERO("test_ggg init");
        */
        Clean_database db;
    }

};

TEST(CommandsBase, Init) {
    int ret;
    ret = std::remove(GGG_ENTITIES_PATH);
    EXPECT_TRUE(ret == 0 || (ret == -1 && errno == ENOENT));
    ret = std::remove(GGG_ACCOUNTS_PATH);
    EXPECT_TRUE(ret == 0 || (ret == -1 && errno == ENOENT));
    EXPECT_ZERO("test_ggg init");
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
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | test_ggg add -f-)");
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u2" #:id 2002)' | test_ggg add -f-)");
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u3" #:id 2003)' | test_ggg add -f-)");
    EXPECT_NON_ZERO2(
        R"(echo '(list (make <entity> #:name "u3" #:id 2003))' | test_ggg add -f-)",
        "added the same user twice"
    );
    EXPECT_NON_ZERO2(
        R"(echo '(list (make <entity> #:name "uu" #:id 2001))' | test_ggg add -f-)",
        "added a user with the same UID twice"
    );
    EXPECT_NON_ZERO2(
        R"(echo '(list (make <entity> #:name "u1" #:id 2222))' | test_ggg add -f-)",
        "added a user with the same USERNAME twice"
    );
}

TEST_F(Commands, Remove) {
    EXPECT_NON_ZERO("test_ggg rm u1");
    EXPECT_OUTPUT("", "test_ggg find u1");
    //EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 3001)' | test_ggg add -f-)");
    //EXPECT_OUTPUT("u1\n", "test_ggg find u1");
    //EXPECT_ZERO("test_ggg rm u1");
    //EXPECT_OUTPUT("", "test_ggg find u1");
    //EXPECT_ZERO("test_ggg rm u1");
}

TEST_F(Commands, Info) {
    EXPECT_NON_ZERO("test_ggg info u1");
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | test_ggg add -f-)");
    EXPECT_ZERO("test_ggg info u1");
    EXPECT_ZERO("test_ggg rm u1");
    EXPECT_NON_ZERO("test_ggg info u1");
    EXPECT_NON_ZERO("test_ggg info donotexist");
}

//TEST_F(Commands, GetEnt) {
//	EXPECT_NON_ZERO("getent passwd u1");
//	EXPECT_NON_ZERO("getent group u1");
//	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | test_ggg add -f-");
//	EXPECT_OUTPUT("u1::2001:2001:U1:/home/u1:/bin/sh\n", "getent passwd u1");
//	EXPECT_OUTPUT("u1::2001:2001:U1:/home/u1:/bin/sh\n", "LANG=donotexist getent passwd u1");
//	EXPECT_OUTPUT("u1::2001:\n", "getent group u1");
//	EXPECT_OUTPUT("u1\n", "getent group u1 | cut -d: -f1");
//	EXPECT_OUTPUT("2001\n", "getent group u1 | cut -d: -f3");
//	EXPECT_ZERO("test_ggg rm u1");
//	EXPECT_NON_ZERO("getent passwd u1");
//	EXPECT_NON_ZERO("getent group u1");
//}

TEST_F(Commands, Find) {
    EXPECT_OUTPUT("", "test_ggg find");
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | test_ggg add -f-)");
    EXPECT_OUTPUT("u1\n", "test_ggg find");
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u2" #:id 2002)' | test_ggg add -f-)");
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u3" #:id 2003)' | test_ggg add -f-)");
    EXPECT_OUTPUT("u1\n", "test_ggg find u1");
    EXPECT_OUTPUT("u1\n", "test_ggg find U1");
    EXPECT_OUTPUT("u2\n", "test_ggg find u2");
    EXPECT_OUTPUT("u3\n", "test_ggg find U3");
    EXPECT_OUTPUT("u1\nu2\nu3\n", "test_ggg find u | sort");
    EXPECT_OUTPUT("u1\nu2\n", "test_ggg find 'u[12]' | sort");
    EXPECT_OUTPUT("", "test_ggg find donotexist");
    EXPECT_ZERO("test_ggg rm u1");
    EXPECT_OUTPUT("", "test_ggg find u1");
}

//TEST_F(Commands, FindOutputMatchesGetEntOutput) {
//	EXPECT_ZERO("echo 'u1:2001:U1:/home/u1:/bin/sh' | test_ggg add -f-");
//	auto result1 = execute_command("test_ggg find -t u1");
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
//		tmp << "' | test_ggg add -f-";
//		std::clog << "tmp.str()=" << tmp.str() << std::endl;
//		EXPECT_ZERO(tmp.str().data());
//	}
//	EXPECT_ZERO("test_ggg duplicates");
//	auto result = execute_command("test_ggg duplicates");
//	EXPECT_EQ(0, result.first.exit_code());
//	EXPECT_TRUE(result.second.empty());
//}


TEST_F(Commands, GroupsAndMembers) {
    EXPECT_ZERO(R"(echo '(make <entity> #:name "u1" #:id 2001)' | test_ggg add -f-)");
    EXPECT_ZERO(R"(echo '(make <entity> #:name "g1" #:id 3001)' | test_ggg add -f-)");
    EXPECT_OUTPUT("", "test_ggg groups u1");
    EXPECT_OUTPUT("", "test_ggg members g1");
    EXPECT_ZERO("test_ggg tie u1 g1");
    EXPECT_OUTPUT("g1\n", "test_ggg groups u1");
    EXPECT_OUTPUT("u1\n", "test_ggg members g1");
    EXPECT_ZERO("test_ggg untie u1");
    EXPECT_OUTPUT("", "test_ggg groups u1");
    EXPECT_OUTPUT("", "test_ggg members g1");
    EXPECT_NON_ZERO("test_ggg groups donotexist");
    EXPECT_NON_ZERO("test_ggg members donotexist");
}

TEST_F(Commands, Version) {
    EXPECT_OUTPUT(GGG_VERSION "\n", "test_ggg version");
    EXPECT_OUTPUT(GGG_VERSION "\n", "test_ggg --version");
    EXPECT_OUTPUT(GGG_VERSION "\n", "test_ggg -v");
}

TEST_F(Commands, Edit) {
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u1" #:id 2001 #:description "N" #:home "H" #:shell "S")')");
    EXPECT_OUTPUT("u1:x:2001:2001:N:H:S\n", "test_ggg find -o passwd u1");
    EXPECT_ZERO(R"(test_ggg edit -e '(make <entity> #:name "u1" #:id 2001 #:description "Nx" #:home "H" #:shell "S")')");
    EXPECT_OUTPUT("u1:x:2001:2001:Nx:H:S\n", "test_ggg find -o passwd u1");
    EXPECT_ZERO(R"(test_ggg edit -e '(make <entity> #:name "u1" #:id 2001 #:description "Nx" #:home "Hx" #:shell "S")')");
    EXPECT_OUTPUT("u1:x:2001:2001:Nx:Hx:S\n", "test_ggg find -o passwd u1");
    EXPECT_ZERO(R"(test_ggg edit -e '(make <entity> #:name "u1" #:id 2001 #:description "Nx" #:home "Hx" #:shell "Sx")')");
    EXPECT_OUTPUT("u1:x:2001:2001:Nx:Hx:Sx\n", "test_ggg find -o passwd u1");
    EXPECT_NON_ZERO2(
        R"(test_ggg edit -e '(make <entity> #:name "u1" #:id 2222 #:description "Nx" #:home "Hx" #:shell "Sx")')",
        "changed entity id but it is not allowed"
    );
    EXPECT_OUTPUT("u1:x:2001:2001:Nx:Hx:Sx\n", "test_ggg find -o passwd u1");
    EXPECT_ZERO(R"(test_ggg edit -e '(make <entity> #:name "u2" #:id 2001 #:description "Nx" #:home "Hx" #:shell "Sx")')");
    EXPECT_OUTPUT("", "test_ggg find -o passwd u1");
    EXPECT_OUTPUT("u2:x:2001:2001:Nx:Hx:Sx\n", "test_ggg find -o passwd u2");
}

TEST_F(Commands, Locale) {
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u1" #:id 2001 #:description "U1")')");
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u2" #:id 2002 #:description "Ю2")')");
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u3" #:id 2003 #:description "U3")')");
    EXPECT_ZERO(R"(test_ggg add -t account -e '(make <account> #:name "u1")')");
    EXPECT_ZERO(R"(test_ggg add -t account -e '(make <account> #:name "u2")')");
    EXPECT_ZERO(R"(test_ggg add -t account -e '(make <account> #:name "u3")')");
    EXPECT_ZERO("test_ggg lock u1 u2 u3");
    EXPECT_OUTPUT("u1\nu2\nu3\n", "test_ggg locked");
    EXPECT_OUTPUT("u1\nu2\nu3\n", "env LC_ALL=C test_ggg locked");
}

TEST_F(Commands, TiesAndHierarchies) {
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u1" #:id 2001 #:description "U1")')");
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u2" #:id 2002 #:description "Ю2")')");
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u3" #:id 2003 #:description "U3")')");
    EXPECT_ZERO("test_ggg attach u1 u2");
    EXPECT_NON_ZERO2("test_ggg attach u1 u2", "attached same entities multiple times");
    EXPECT_ZERO("test_ggg tie u1 u2");
    EXPECT_ZERO("test_ggg detach u1");
    EXPECT_ZERO("test_ggg tie u1 u2");
    EXPECT_NON_ZERO2("test_ggg attach u1 u2", "attached tied entities");
    EXPECT_ZERO("test_ggg untie u1 u2");
    EXPECT_ZERO("test_ggg attach u1 u2");
}

TEST_F(Commands, PublicKeys) {
    EXPECT_ZERO(R"(test_ggg add -e '(make <entity> #:name "u1" #:id 2001 #:description "U1")')");
    EXPECT_ZERO(R"(test_ggg add -t account -e '(make <account> #:name "u1")')");
    EXPECT_ZERO(R"(test_ggg add -t public-key \
                                -e '(make <public-key>
                                          #:name "u1"
                                          #:options ""
                                          #:type "ssh-rsa"
                                          #:key "AAAA"
                                          #:comment "hello")')");
    EXPECT_ZERO(R"(test_ggg add -t public-key \
                                -e '(make <public-key>
                                          #:name "u1"
                                          #:options ""
                                          #:type "ssh-rsa"
                                          #:key "BBBB"
                                          #:comment "hello")')");
    EXPECT_OUTPUT("ssh-rsa AAAA hello\nssh-rsa BBBB hello\n", R"(test_ggg-public-keys u1)");
}

int
main(int argc, char* argv[]) {
    const char* ggg_path = getenv("GGG_PATH");
    if (!ggg_path) { return 77; }
    if (const char* path = getenv("PATH")) {
        std::string new_path = ggg_path;
        new_path += ':';
        new_path += path;
        ::setenv("PATH", new_path.data(), 1);
    } else {
        ::setenv("PATH", ggg_path, 1);
    }
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new DaemonEnvironment);
    return RUN_ALL_TESTS();
}
