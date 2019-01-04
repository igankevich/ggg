#ifndef SHOW_VERSION_HH
#define SHOW_VERSION_HH

#include "command.hh"

namespace ggg {

	class Show_version: public Command {

	private:
		enum struct Field {
			Revision,
			Version,
			Date
		};
		Field _field = Field::Version;

	public:
		void parse_arguments(int argc, char* argv[]) override;
		void execute() override;
		void print_usage() override;
	};

}

#endif // SHOW_VERSION_HH
