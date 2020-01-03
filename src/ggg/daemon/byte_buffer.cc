#include <cstring>

#include <ggg/daemon/byte_buffer.hh>

auto
ggg::byte_buffer::write(const_pointer src, size_type n) -> size_type {
    while (n > remaining()) { grow(); this->_limit = size(); }
    std::memcpy(data()+position(), src, n);
    this->_position += n;
    return n;
}

auto
ggg::byte_buffer::read(pointer dst, size_type n) -> size_type {
    auto r = remaining();
    if (n > r) { n = r; }
    std::memcpy(dst, data()+position(), n);
    return n;
}

void
ggg::byte_buffer::flip() {
    this->_limit = this->_position;
    this->_position = 0;
}

void
ggg::byte_buffer::compact() {
    auto n = remaining();
    std::memmove(data(), data()+position(), n);
    this->_position = n;
    this->_limit = size();
}

void
ggg::byte_buffer::clear() {
    this->_position = 0;
    this->_limit = size();
}
