#include <unistdx/base/log_message>

#include <ggg/daemon/remote_client.hh>

void
ggg::Remote_client::process(const sys::epoll_event& event) {
    if (starting() && !event.bad()) {
        sys::log_message("server", "accept _", this->_address);
        this->state(State::Started);
    }
    if (started() && event.bad()) {
        this->state(State::Stopping);
    }
    if (started()) {
        if (event.in()) {
        }
        if (event.out()) {
        }
    }
}

