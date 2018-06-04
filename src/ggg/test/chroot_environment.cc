#include "chroot_environment.hh"

#include <sys/mount.h>

#include <sstream>

#include <unistdx/base/check>
#include <unistdx/fs/canonical_path>
#include <unistdx/fs/path>

#include <ggg/config.hh>

void
ChrootEnvironment::SetUp() {
	try {
	UNISTDX_CHECK(::unshare(CLONE_NEWNS));
	sys::path overlay("overlay");
	sys::path myroot(overlay, "myroot");
	sys::path newroot(overlay, "newroot");
	sys::path workdir(overlay, "workdir");
	std::stringstream opts;
	opts << "lowerdir=/,upperdir=" << myroot << ",workdir=" << workdir;
	UNISTDX_CHECK(::mount("none", "/", nullptr, MS_REC|MS_PRIVATE, nullptr))
	UNISTDX_CHECK(
		::mount("overlay", newroot, "overlay", 0, opts.str().data())
	);
	UNISTDX_CHECK(::chroot(newroot));
	UNISTDX_CHECK(::chdir("/"));
	UNISTDX_CHECK(::mount("devtmpfs", "/dev", "devtmpfs", 0, nullptr));
	UNISTDX_CHECK(::mount("devpts", "/dev/pts", "devpts", 0, nullptr));
	UNISTDX_CHECK(::mount("proc", "/proc", "proc", 0, nullptr));
	} catch (const sys::bad_call& err) {
		std::cerr << err << std::endl;
	}
}

void
ChrootEnvironment::TearDown() {
	ASSERT_EQ(0, ::system("rm -rf " GGG_ROOT));
}

