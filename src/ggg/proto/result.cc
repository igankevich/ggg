#include <ggg/proto/result.hh>

void ggg::Result::read(sys::byte_buffer& buf) {
    buf.read(this->_code);
}

void ggg::Result::write(sys::byte_buffer& buf) {
    buf.write(this->_code);
}
