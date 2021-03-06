#ifndef GGG_PROTO_PROTOCOL_HH
#define GGG_PROTO_PROTOCOL_HH

#include <unistdx/base/byte_buffer>
#include <unistdx/base/log_message>
#include <unistdx/base/types>
#include <unistdx/net/socket>

#include <ggg/proto/kernel.hh>

namespace ggg {

    class Protocol {

    public:
        static constexpr const sys::u32 version = 1;

        enum class Command: sys::u32 {
            Unspecified = 0,
            PAM_kernel = 1,
            NSS_kernel = 2,
            Size = 3,
        };

        struct Frame {
            sys::u32 size = 0;
            sys::u32 version = ::ggg::Protocol::version;
            Command command{};
        };

    };

    class Server_protocol: public Protocol {

    private:
        bool _log_pam = true;
        bool _log_nss = true;

    public:
        void process(sys::socket& sock, sys::byte_buffer& in, sys::byte_buffer& out);

        inline void log_pam(bool rhs) { this->_log_pam = rhs; }
        inline void log_nss(bool rhs) { this->_log_nss = rhs; }

        template <class ... Args>
        inline void
        log(const char* message, const Args& ... args) const {
            sys::log_message("server", message, args...);
        }

    };

    class Client_protocol: public Protocol {

    public:
        void process(Kernel* kernel, Command command);

        template <class ... Args>
        inline void
        log(const char* message, const Args& ... args) const {
            sys::log_message("client", message, args...);
        }

    };

    struct buffer_guard {
        sys::byte_buffer& _buffer;
        inline explicit buffer_guard(sys::byte_buffer& buffer): _buffer(buffer) {
            this->_buffer.flip();
        }
        inline ~buffer_guard() { this->_buffer.compact(); }
    };

}

#endif // vim:filetype=cpp
