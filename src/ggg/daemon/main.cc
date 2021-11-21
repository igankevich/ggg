#include <iostream>

#include <unistdx/ipc/signal>

#include <ggg/config.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/proto/local_server.hh>
#include <ggg/proto/pipeline.hh>
#include <ggg/proto/protocol.hh>

// TODO implement tcp server
int main(int argc, char* argv[]) {
    using namespace ggg;
    int ret = EXIT_FAILURE;
    sys::this_process::ignore_signal(sys::signal::broken_pipe);
    try {
        sys::socket_address bind_address(GGG_BIND_ADDRESS);
        Pipeline ppl;
        ppl.add(new Local_server(bind_address));
        ppl.run();
        ret = EXIT_SUCCESS;
    } catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
    }
    return ret;
}
