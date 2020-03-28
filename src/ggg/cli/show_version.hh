#ifndef GGG_CLI_SHOW_VERSION_HH
#define GGG_CLI_SHOW_VERSION_HH

#include <ggg/cli/command.hh>

namespace ggg {

    class Show_version: public Command {

    public:
        void execute() override;
    };

}

#endif // vim:filetype=cpp
