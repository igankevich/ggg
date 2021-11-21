#ifndef GGG_PROTO_PIPELINE_HH
#define GGG_PROTO_PIPELINE_HH

#include <memory>
#include <unordered_map>

#include <unistdx/base/log_message>
#include <unistdx/io/poller>

#include <ggg/proto/connection.hh>

namespace ggg {

    struct No_lock { void lock() {} void unlock() {} };

    class Pipeline {

    public:
        using clock_type = Network_connection::clock_type;
        using time_point = Network_connection::time_point;
        using duration = Network_connection::duration;
        using lock_type = No_lock;
        using connection_ptr = std::unique_ptr<Network_connection>;
        using connection_table = std::unordered_map<sys::fd_type,connection_ptr>;

    private:
        sys::event_poller _poller;
        connection_table _connections;

    public:

        void add(Network_connection* connection, sys::event events=sys::event::in);

        inline void run() {
            lock_type lock;
            this->_poller.wait(
                lock,
                [this] () { this->process_events(); return false; }
            );
        }

    private:

        void process_events();

        template <class ... Args>
        inline void
        log(const char* message, const Args& ... args) const {
            sys::log_message("pipeline", message, args...);
        }

    };

}

#endif // vim:filetype=cpp
