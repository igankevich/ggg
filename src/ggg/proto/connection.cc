#include <ggg/proto/connection.hh>

void
ggg::Connection::process(const sys::epoll_event& event) {
    if (initial()) { throw std::logic_error("bad state"); }
    if (starting() && !event.bad()) { state(State::Started); }
    if (started() && event.bad()) { state(State::Stopping); }
    if (stopping()) { state(State::Stopped); }
}
