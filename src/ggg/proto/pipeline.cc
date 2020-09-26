#include <ggg/proto/pipeline.hh>

void
ggg::Pipeline::add(Connection* connection, sys::event events) {
    if (!connection) { throw std::invalid_argument("bad connection"); }
    connection->parent(this);
    auto fd = connection->fd();
    this->_connections[fd] = connection_ptr(connection);
    this->_poller.emplace(fd, events);
    connection->start();
}

void
ggg::Pipeline::process_events() {
    auto pipe_fd = this->_poller.pipe_in();
    for (const auto& event : this->_poller) {
        if (event.fd() == pipe_fd) { continue; }
        auto result = this->_connections.find(event.fd());
        if (result == this->_connections.end()) {
            this->log("bad fd _", event.fd());
            continue;
        }
        auto& connection = *result->second;
        try {
            connection.process(event);
        } catch (const std::exception& err) {
            if (connection.started()) { connection.stop(); }
            this->log("_", err.what());
        }
        if (connection.stopped()) {
            this->_poller.erase(event.fd());
            this->_connections.erase(result);
        }
    }
}
