#include "chroot_environment.hh"
#include "execute_command.hh"

#include <ggg/config.hh>

TEST(Form, Run) {
	ASSERT_EQ(0, ::system("rm -rf " GGG_ROOT));
	ASSERT_EQ(0, ::system("ggg init"));
	ok("echo 'u1:2001:U1:/:/bin/sh' | ggg add -");
	ok(
		R"(echo '
text:1:First name:^[A-Za-z ]+$
text:2:Last name:^[A-Za-zА-Яа-я ]+$
text:3:Username:^[A-Za-z0-9]+$
password:4:Password:^.{6,100}$
set:entropy:1.0
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
	ok("su - u1 -c ggg-form");
}

int
main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new ChrootEnvironment);
	return RUN_ALL_TESTS();
}

