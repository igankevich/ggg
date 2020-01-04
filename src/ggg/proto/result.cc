#include <ggg/proto/result.hh>

void ggg::Result::read(byte_buffer& buf) {
    buf.read(this->_code);
}

void ggg::Result::write(byte_buffer& buf) {
    buf.write(this->_code);
}
