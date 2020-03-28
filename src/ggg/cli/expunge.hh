#ifndef GGG_CLI_EXPUNGE_HH
#define GGG_CLI_EXPUNGE_HH

#include <unordered_set>
#include <vector>

#include <unistdx/ipc/identity>
#include <unistdx/ipc/process>

#include "command.hh"
#include <ggg/core/entity.hh>

namespace ggg {

    struct Process {
        sys::pid_type pid;
        sys::uid_type uid;

        inline
        Process(sys::pid_type p, sys::uid_type u):
        pid(p), uid(u) {}

    };

    class Expunge: public Command {

    private:
        std::unordered_set<sys::uid_type> _uids;
        std::vector<Process> _processes;

    public:
        void parse_arguments(int argc, char* argv[]) override;
        void execute() override;

    private:

        void
        find_expired_entities();

        void
        find_processes();

    };

}

#endif // vim:filetype=cpp
