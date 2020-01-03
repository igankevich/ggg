#ifndef GGG_DAEMON_PROTOCOL_HH
#define GGG_DAEMON_PROTOCOL_HH

#include <unistdx/base/types>

#include <ggg/daemon/byte_buffer.hh>

namespace ggg {

    class Protocol {

    public:
        static constexpr const sys::u32 version = 1;

        enum class Command: sys::u32 {
            Unknown = 0,
            Authenticate = 1,
            Size = 2,
        };

        struct Frame {
            sys::u32 size = 0;
            sys::u32 version = ::ggg::Protocol::version;
            Command command = Command::Unknown;
        };

    };

    class Server_protocol: public Protocol {

    public:
        void read(byte_buffer& buf);
        void write(byte_buffer& buf);

    };

}

#endif // vim:filetype=cpp
