#ifndef GGG_PROTO_CONNECTION_HH
#define GGG_PROTO_CONNECTION_HH

#include <chrono>
#include <iosfwd>

#include <unistdx/net/socket>
#include <unistdx/io/epoll_event>

#include <ggg/proto/forward.hh>

namespace ggg {

    class Network_connection {

    public:
        using clock_type = std::chrono::system_clock;
        using time_point = clock_type::time_point;
        using duration = clock_type::duration;

    public:
        enum class State {Initial, Starting, Started, Stopping, Stopped};

    private:
        Pipeline* _parent = nullptr;
        State _state = State::Initial;

    protected:
        sys::socket _socket;

    public:

        Network_connection() = default;
        inline explicit Network_connection(sys::family_type family): _socket(family) {}
        virtual ~Network_connection() {}

        virtual void
        process(const sys::epoll_event& event);

        inline void parent(Pipeline* rhs) { this->_parent = rhs; }
        inline Pipeline* parent() { return this->_parent; }
        inline const Pipeline* parent() const { return this->_parent; }
        inline sys::fd_type fd() const noexcept { return this->_socket.fd(); }
        inline State state() const { return this->_state; }
        inline void state(State s) { this->_state = s; }
        inline bool initial() const { return this->_state == State::Initial; }
        inline bool starting() const { return this->_state == State::Starting; }
        inline bool started() const { return this->_state == State::Started; }
        inline bool stopping() const { return this->_state == State::Stopping; }
        inline bool stopped() const { return this->_state == State::Stopped; }

        inline void start() {
            if (this->_state != State::Initial) { throw std::logic_error("bad state"); }
            this->_state = State::Starting;
        }

        inline void stop() {
            if (this->_state != State::Started) { throw std::logic_error("bad state"); }
            this->_state = State::Stopping;
        }

        inline sys::port_type port() const {
            return sys::socket_address_cast<sys::ipv4_socket_address>(this->_socket.bind_addr()).port();
        }

    };

    const char* state_to_string(Network_connection::State s);
    std::ostream& operator<<(std::ostream& out, Network_connection::State rhs);

}

#endif // vim:filetype=cpp
