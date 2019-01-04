#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/native.hh>
#include <ggg/cli/show_members.hh>

void
ggg::Show_members::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Show_members::execute() {
	Database db(GGG_ENTITIES_PATH);
	for (const std::string& a : this->args()) {
		auto rstr = db.find_child_entities(a.data());
		user_iterator first(rstr), last;
		while (first != last) {
			this->_result.insert(*first);
			++first;
		}
	}
	Show_base::execute();
}
