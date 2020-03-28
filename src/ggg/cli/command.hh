#ifndef GGG_CLI_COMMAND_HH
#define GGG_CLI_COMMAND_HH

#include <string>
#include <vector>

namespace ggg {

    class Command {

    public:
        typedef std::vector<std::string> args_type;
        typedef args_type::const_iterator args_iterator;

    protected:
        std::string _prefix;
        args_type _args;
        bool _verbose = true;

    public:
        Command() = default;
        virtual ~Command() = default;
        Command(const Command&) = delete;
        Command(Command&&) = delete;
        Command& operator=(const Command&) = delete;

        virtual void parse_arguments(int argc, char* argv[]);
        virtual void execute() = 0;
        virtual void print_usage();

        inline void
        prefix(const std::string& rhs) {
            this->_prefix = rhs;
        }

        inline const std::string&
        prefix() const noexcept {
            return this->_prefix;
        }

        inline bool
        verbose() const noexcept {
            return this->_verbose;
        }

        inline args_iterator
        args_begin() const noexcept {
            return this->_args.begin();
        }

        inline args_iterator
        args_end() const noexcept {
            return this->_args.end();
        }

        inline const args_type&
        args() const noexcept {
            return this->_args;
        }

        inline bool
        is_batch() const noexcept {
            return this->_args.size() == 1 && *this->_args.begin() == "-";
        }

        void
        remove_duplicate_arguments();

    };

}

#endif // vim:filetype=cpp
