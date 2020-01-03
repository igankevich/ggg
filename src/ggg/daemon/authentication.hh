#ifndef GGG_DAEMON_AUTHENTICATION_HH
#define GGG_DAEMON_AUTHENTICATION_HH

#include <ggg/daemon/kernel.hh>

namespace ggg {

    class Authenticate: public Kernel {
    public:
        void run() override;
        void read(byte_buffer& buf) override;
        void write(byte_buffer& buf) override;
    };

}

#endif // vim:filetype=cpp
