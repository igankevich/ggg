#ifndef GGG_CLI_MESSAGES_HH
#define GGG_CLI_MESSAGES_HH

#include <ggg/cli/command.hh>

namespace ggg {

	class Messages: public Command {

    private:
        enum class Option { Default, Help, Size };

    private:
        Option _option = Option::Default;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

	class Rotate_messages: public Command {

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp
