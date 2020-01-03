#ifndef GGG_DAEMON_KERNEL_HH
#define GGG_DAEMON_KERNEL_HH

#include <ggg/daemon/byte_buffer.hh>

namespace ggg {

    class Kernel {
    public:
        Kernel() = default;
        virtual ~Kernel() = default;
        virtual void run() = 0;
        virtual void read(byte_buffer& buf) = 0;
        virtual void write(byte_buffer& buf) = 0;
    };

}

#endif // vim:filetype=cpp
