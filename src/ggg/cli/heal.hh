#ifndef GGG_CLI_HEAL_HH
#define GGG_CLI_HEAL_HH

#include <ostream>
#include <string>

#include <ggg/cli/command.hh>

namespace ggg {

    class Heal: public Command {

    public:
        void execute() override;

    };

}

#endif // vim:filetype=cpp
