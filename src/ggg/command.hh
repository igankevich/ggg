#ifndef GGG_COMMAND_HH
#define GGG_COMMAND_HH

#include <string>

namespace ggg {

	class Command {

	private:
		std::string _prefix;

	public:
		Command() = default;
		virtual ~Command() = default;
		Command(const Command&) = delete;
		Command(Command&&) = delete;
		Command& operator=(const Command&) = delete;

		virtual void execute(int argc, char* argv[]) = 0;
		virtual void print_usage();

		inline void
		prefix(const std::string& rhs) {
			this->_prefix = rhs;
		}

		inline const std::string&
		prefix() const noexcept {
			return this->_prefix;
		}

	};

}

#endif // GGG_COMMAND_HH
