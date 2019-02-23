#include <nss.h>

#include <cstdlib>

#include <gtest/gtest.h>
#include <unistdx/base/check>

int main(int argc, char* argv[]) {
	const char* service = std::getenv("GGG_NSS_SERVICE");
	UNISTDX_CHECK(::__nss_configure_lookup(service, "ggg"));
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
