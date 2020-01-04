#include <cstring>

#include <ggg/proto/byte_buffer.hh>

auto
ggg::byte_buffer::write(const_pointer src, size_type n) -> size_type {
    while (n > remaining()) { grow(); this->_limit = size(); }
    std::memcpy(data()+position(), src, n);
    this->_position += n;
    return n;
}

auto
ggg::byte_buffer::read(pointer dst, size_type n) -> size_type {
    if (n > remaining()) { throw std::range_error("ggg::byte_buffer::read"); }
    std::memcpy(dst, data()+position(), n);
    this->_position += n;
    return n;
}

void
ggg::byte_buffer::peek(pointer dst, size_type n) {
    std::memcpy(dst, data()+position(), n);
}

void
ggg::byte_buffer::bump(size_type n) {
    while (n > remaining()) { grow(); this->_limit = size(); }
    this->_position += n;
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
    std::memset(data(), 0, size());
}
