#include <ggg/ggg/show_locked.hh>

#include <iostream>

#include <ggg/config.hh>
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
	Database db(GGG_DATABASE_PATH);
	auto rstr = db.locked_entities();
	user_iterator first(rstr), last;
	while (first != last) {
		this->_result.insert(*first);
		++first;
	}
	Show_base::execute();
}

