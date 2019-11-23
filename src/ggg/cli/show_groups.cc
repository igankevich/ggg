#include <iostream>

#include <ggg/core/database.hh>
#include <ggg/core/native.hh>
#include <ggg/cli/show_groups.hh>

void
ggg::Show_groups::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Show_groups::execute() {
	Database db(Database::File::Entities);
	for (const std::string& a : this->args()) {
		this->_result = db.find_parent_entities(a.data());
        Show_base::execute();
	}
}

