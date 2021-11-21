#include <ggg/proto/connection.hh>

void
ggg::Network_connection::process(const sys::epoll_event& event) {
    if (initial()) { throw std::logic_error("bad state"); }
    if (starting()) { state(State::Started); }
    if (event.bad()) { state(State::Stopping); }
    if (stopping()) { state(State::Stopped); }
}

const char* ggg::state_to_string(Network_connection::State s) {
    switch (s) {
        case Network_connection::State::Initial: return "Initial";
        case Network_connection::State::Starting: return "Starting";
        case Network_connection::State::Started: return "Started";
        case Network_connection::State::Stopping: return "Stopping";
        case Network_connection::State::Stopped: return "Stopped";
        default: return nullptr;
    }
}

std::ostream& ggg::operator<<(std::ostream& out, Network_connection::State rhs) {
    const char* str = state_to_string(rhs);
    if (str) { out << str; } else { out << "unknown(" << int(rhs) << ')'; }
    return out;
}
