#include <nss.h>

#include <cstdlib>

#include <ggg/proto/daemon_environment.hh>
#include <gtest/gtest.h>
#include <unistdx/base/check>

int main(int argc, char* argv[]) {
    std::string services = GGG_NSS_SERVICE;
    services += ' ';
    std::string service;
    for (auto ch : services) {
        if (ch == ' ') {
            std::clog << "service=" << service << std::endl;
            UNISTDX_CHECK(::__nss_configure_lookup(service.data(), "test_ggg"));
            service.clear();
        } else {
            service += ch;
        }
    }
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new DaemonEnvironment);
    return RUN_ALL_TESTS();
}
