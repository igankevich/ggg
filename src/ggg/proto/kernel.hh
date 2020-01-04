#ifndef GGG_PROTO_KERNEL_HH
#define GGG_PROTO_KERNEL_HH

#include <unistdx/net/socket>

#include <ggg/proto/byte_buffer.hh>

namespace ggg {

    class Kernel {

    private:
        sys::u32 _result = 0;
        sys::user_credentials _client_credentials{};

    public:
        Kernel() = default;
        virtual ~Kernel() = default;
        virtual void run() = 0;
        virtual void read(byte_buffer& buf) = 0;
        virtual void write(byte_buffer& buf) = 0;

        inline void result(sys::u32 rhs) { this->_result = rhs; }
        inline sys::u32 result() const { return this->_result; }
        inline const sys::user_credentials& client_credentials() const {
            return this->_client_credentials;
        }
        inline void client_credentials(const sys::user_credentials& rhs) {
            this->_client_credentials = rhs;
        }

    };

}

#endif // vim:filetype=cpp