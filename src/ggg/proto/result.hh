#ifndef GGG_PROTO_RESULT_HH
#define GGG_PROTO_RESULT_HH

#include <ggg/proto/kernel.hh>

namespace ggg {

    class Result: public Kernel {

    public:
        using Type = sys::i32;

    private:
        Type _code = 0;

    public:
        Result() = default;
        inline explicit Result(Type code): _code(code) {}
        inline Type code() const { return this->_code; }
        inline void code(Type rhs) { this->_code = rhs; }
        void run() override {}
        void read(sys::byte_buffer& buf) override;
        void write(sys::byte_buffer& buf) override;

    };

}

#endif // vim:filetype=cpp
