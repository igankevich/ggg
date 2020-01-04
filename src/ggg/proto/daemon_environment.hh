#ifndef GGG_PROTO_DAEMON_ENVIRONMENT_HH
#define GGG_PROTO_DAEMON_ENVIRONMENT_HH

#include <gtest/gtest.h>

#include <unistdx/ipc/process>

class DaemonEnvironment: public ::testing::Environment {
private:
    sys::process _daemon;
public:
    void SetUp() override;
    void TearDown() override;
};

#endif // vim:filetype=cpp
