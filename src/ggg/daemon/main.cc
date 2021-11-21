#include <iostream>
#include <sstream>

#include <unistdx/ipc/signal>
#include <unistdx/net/socket_address>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/proto/authentication.hh>
#include <ggg/proto/local_server.hh>
#include <ggg/proto/pipeline.hh>
#include <ggg/proto/protocol.hh>

void usage(std::ostream& out, const char* name) {
    out << name << " [-hv] [--help] [--version] address...\n";
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        if (argv[1] == std::string("--help")) {
            usage(std::cout, argv[0]);
            std::exit(0);
        }
        if (argv[1] == std::string("--help")) {
            std::cout << GGG_VERSION "\n";
            std::exit(0);
        }
    }
    int opt = 0;
    while ((opt = ::getopt(argc, argv, "hv"))) {
        switch (opt) {
            case 'h':
                usage(std::cout, argv[0]);
                std::exit(0);
                break;
            case 'v':
                std::cout << GGG_VERSION "\n";
                std::exit(0);
                break;
            default:
                usage(std::cerr, argv[0]);
                std::exit(1);
                break;
        }
    }
    using namespace ggg;
    int ret = EXIT_FAILURE;
    sys::this_process::ignore_signal(sys::signal::broken_pipe);
    try {
        Database entities(Database::File::Entities, Database::Flag::Read_only);
        Database accounts(Database::File::Accounts, Database::Flag::Read_write);
        Pipeline ppl;
        ppl.add(new Local_server(sys::socket_address(GGG_BIND_ADDRESS), entities, accounts));
        for (int i=::optind; i<argc; ++i) {
            sys::socket_address address;
            std::stringstream tmp;
            tmp << argv[i];
            tmp >> address;
            ppl.add(new Local_server(address, entities, accounts));
        }
        ppl.run();
        ret = EXIT_SUCCESS;
    } catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
    }
    return ret;
}
