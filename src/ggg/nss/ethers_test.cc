#include <netinet/ether.h>
#include <nss.h>

#include <gtest/gtest.h>

#include <ggg/core/machine.hh>
#include <ggg/test/clean_database.hh>

using namespace ggg;

/*
TEST(ethers, empty) {
    { Clean_database db; }
    auto* addr = ::ether_aton("x.domain");
    ASSERT_EQ(nullptr, addr) << sys::ethernet_address(addr->ether_addr_octet);
}

TEST(ethers, ether_aton) {
    {
        Clean_database db;
        db.insert(Machine("x", ip_address({1,1,1,1}), {1,1,1,1,1,1}));
        auto st = db.find_host("x");
        if (st.step() == sqlite::errc::done) {
            std::clog << "123=" << 123 << std::endl;
        }
    }
    ether_addr addr;
    ASSERT_EQ(0,::ether_hostton("x", &addr));
    sys::ethernet_address ea(addr.ether_addr_octet);
}

*/
