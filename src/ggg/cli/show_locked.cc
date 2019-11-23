#include <ggg/cli/show_locked.hh>

#include <iostream>

#include <ggg/core/database.hh>
#include <ggg/core/native.hh>

void
ggg::Show_locked::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
	if (!this->_args.empty()) {
		throw std::invalid_argument("please, do not specify entity names");
	}
}

void
ggg::Show_locked::execute() {
	Database db(Database::File::All);
	this->_result = db.locked_entities();
	Show_base::execute();
}

