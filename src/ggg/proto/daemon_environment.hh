#ifndef GGG_PROTO_DAEMON_ENVIRONMENT_HH
#define GGG_PROTO_DAEMON_ENVIRONMENT_HH

#include <gtest/gtest.h>

#include <unistdx/ipc/process>

#include <ggg/core/system_log.hh>

class DaemonEnvironment: public ::testing::Environment {

private:
    sys::process _daemon;
    #if defined(GGG_TEST)
    sys::log _log{nullptr, sys::log::facilities::user, sys::log::options::stderr};
    #endif

public:
    void SetUp() override;
    void TearDown() override;

};

#endif // vim:filetype=cpp
