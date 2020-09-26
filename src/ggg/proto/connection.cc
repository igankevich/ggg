#include <ggg/proto/connection.hh>

void
ggg::Connection::process(const sys::epoll_event& event) {
    if (initial()) { throw std::logic_error("bad state"); }
    if (starting()) { state(State::Started); }
    if (event.bad()) { state(State::Stopping); }
    if (stopping()) { state(State::Stopped); }
}

const char* ggg::state_to_string(Connection::State s) {
    switch (s) {
        case Connection::State::Initial: return "Initial";
        case Connection::State::Starting: return "Starting";
        case Connection::State::Started: return "Started";
        case Connection::State::Stopping: return "Stopping";
        case Connection::State::Stopped: return "Stopped";
        default: return nullptr;
    }
}

std::ostream& ggg::operator<<(std::ostream& out, Connection::State rhs) {
    const char* str = state_to_string(rhs);
    if (str) { out << str; } else { out << "unknown(" << int(rhs) << ')'; }
    return out;
}
