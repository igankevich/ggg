#ifndef GGG_PROTO_PROTOCOL_HH
#define GGG_PROTO_PROTOCOL_HH

#include <unistdx/base/types>
#include <unistdx/net/socket>

#include <ggg/proto/byte_buffer.hh>
#include <ggg/proto/kernel.hh>

namespace ggg {

    class Protocol {

    public:
        static constexpr const sys::u32 version = 1;

        enum class Command: sys::u32 {
            Result = 0,
            PAM_kernel = 1,
            Size = 2,
        };

        struct Frame {
            sys::u32 size = 0;
            sys::u32 version = ::ggg::Protocol::version;
            Command command{};
        };

    };

    class Server_protocol: public Protocol {

    public:
        void process(sys::socket& sock, byte_buffer& in, byte_buffer& out);

    };

    class Client_protocol: public Protocol {

    public:
        sys::u32 process(Kernel* kernel);

    };

}

#endif // vim:filetype=cpp
