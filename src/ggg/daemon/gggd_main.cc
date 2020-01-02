#include <ggg/config.hh>
#include <ggg/daemon/local_server.hh>
#include <ggg/daemon/pipeline.hh>

int main(int argc, char* argv[]) {
    using namespace ggg;
    int ret = EXIT_FAILURE;
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
