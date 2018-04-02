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

#include <ggg/config.hh>

#include <gtest/gtest.h>

class VirtualEnvironment: public ::testing::Environment {

public:

	void
	SetUp() override {
//		UNISTDX_CHECK(::unshare(CLONE_NEWNS));
		sys::canonical_path overlay(std::getenv("GGG_OVERLAY"));
		sys::path myroot(overlay, "myroot");
		sys::path newroot(overlay, "newroot");
		sys::path workdir(overlay, "workdir");
		std::stringstream options;
		options << "lowerdir=/,upperdir=" << myroot << ",workdir=" << workdir;
//		UNISTDX_CHECK(::mount(nullptr, "/", nullptr, MS_REC|MS_PRIVATE, nullptr));
		UNISTDX_CHECK(::mount("overlay", newroot, "overlay", 0, options.str().data()));
		UNISTDX_CHECK(::chroot(newroot));
		UNISTDX_CHECK(::system("rm -rf " GGG_ROOT));
		/*
		sys::idirtree tree(root);
		std::for_each(
			sys::idirtree_iterator<sys::pathentry>(tree),
			sys::idirtree_iterator<sys::pathentry>(),
			[&root] (const sys::pathentry& entry) {
			    if (sys::get_file_type(entry) == sys::file_type::regular) {
					sys::path src(entry.getpath());
					sys::path target;
					if (src.compare(0, root.length(), root) == 0) {
						target = src.substr(root.length(), src.length()-root.length());
					}
					std::clog
						<< "mount " << src << ' '
						<< target << std::endl;
					UNISTDX_CHECK(::mount(src, target, nullptr, MS_BIND, nullptr));
				}
			}
		);
		*/
	}

};



TEST(NSSTEST, ggg_init) {
	EXPECT_EQ(0, ::system("ggg init"));
}

TEST(NSSTEST, Test2) {
	std::clog << __func__ << std::endl;
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new VirtualEnvironment);
	return RUN_ALL_TESTS();
}
