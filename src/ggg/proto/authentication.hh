#ifndef GGG_PROTO_AUTHENTICATION_HH
#define GGG_PROTO_AUTHENTICATION_HH

#include <string>

#include <ggg/proto/kernel.hh>
#include <ggg/sec/secure_string.hh>

namespace ggg {

    class PAM_kernel: public Kernel {

    public:
        enum PAM {
            Auth = 1<<0,
            Account = 1<<1,
            Password = 1<<2,
            Open_session = 1<<3,
            Close_session = 1<<4,
        };
        enum Error {
            Suspended = 1<<31,
            Expired = 1<<30,
            Inactive = 1<<29,
            Password_expired = 1<<28,
        };

    private:
        secure_string _name;
        secure_string _password;
        secure_string _old_password;
        std::string _service;
        double _min_entropy = 30.0;
        sys::u32 _steps = Auth | Account | Open_session;

    public:
        PAM_kernel() = default;
        inline explicit PAM_kernel(secure_string name, secure_string password):
        _name(std::move(name)), _password(std::move(password)) {}
        inline sys::u32 steps() const { return this->_steps; }
        inline void steps(sys::u32 rhs) { this->_steps = rhs; }
        inline void name(secure_string rhs) { this->_name = std::move(rhs); }
        inline void password(secure_string rhs) { this->_password = std::move(rhs); }
        inline void old_password(secure_string rhs) { this->_old_password = std::move(rhs); }
        inline void min_entropy(double rhs) { this->_min_entropy = rhs; }
        inline void service(std::string rhs) { this->_service = std::move(rhs); }

        void run() override;
        void read(byte_buffer& buf) override;
        void write(byte_buffer& buf) override;

    };

}

#endif // vim:filetype=cpp
