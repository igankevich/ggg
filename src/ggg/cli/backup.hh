#ifndef GGG_GGG_BACKUP_HH
#define GGG_GGG_BACKUP_HH

#include <ggg/cli/command.hh>

namespace ggg {

	class Backup: public Command {

	private:
		const char* _format = "%Y-%m-%d--%H-%M-%S";

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // vim:filetype=cpp


