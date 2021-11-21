#include <unistdx/ipc/process_semaphore>

#include <ggg/config.hh>
#include <ggg/proto/daemon_environment.hh>
#include <ggg/proto/local_server.hh>
#include <ggg/proto/pipeline.hh>

void DaemonEnvironment::SetUp() {
    #if defined(GGG_TEST)
    this->_log.max_level(sys::log::levels::debug);
    #endif
    sys::sysv_semaphore sem;
    this->_daemon = sys::process([&sem] () {
        using namespace ggg;
        sys::socket_address bind_address(GGG_BIND_ADDRESS);
        Pipeline ppl;
        ppl.add(new Local_server(bind_address));
        sem.notify_one();
        ppl.run();
    });
    sem.wait();
}

void DaemonEnvironment::TearDown() {
    this->_daemon.terminate();
    this->_daemon.wait();
}
