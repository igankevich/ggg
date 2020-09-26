#include <unistdx/base/log_message>

#include <ggg/proto/protocol.hh>
#include <ggg/proto/remote_client.hh>

void
ggg::Remote_client::process(const sys::epoll_event& event) {
    Connection::process(event);
    std::clog << "event=" << event << std::endl;
    //if (event.in()) {
        auto nfill = this->_in.fill(this->_socket);
        std::clog << "nfill=" << nfill << std::endl;
        buffer_guard g1(this->_in);
        this->_protocol.process(this->_socket, this->_in, this->_out);
    //}
    buffer_guard g2(this->_out);
    auto nflush = this->_out.flush(this->_socket);
    std::clog << "nflush=" << nflush << std::endl;
}
