#include <nss.h>

#include <cstdlib>

#include <ggg/proto/daemon_environment.hh>
#include <gtest/gtest.h>
#include <unistdx/base/check>

int main(int argc, char* argv[]) {
    const char* service = GGG_NSS_SERVICE;
    UNISTDX_CHECK(::__nss_configure_lookup(service, "ggg"));
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new DaemonEnvironment);
    return RUN_ALL_TESTS();
}
