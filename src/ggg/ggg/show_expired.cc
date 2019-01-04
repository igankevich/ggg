#include "show_expired.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

#include <ggg/config.hh>
#include <ggg/core/native.hh>
#include <ggg/core/database.hh>

void
ggg::Show_expired::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
	if (!this->_args.empty()) {
		throw std::invalid_argument("please, do not specify entity names");
	}
}

void
ggg::Show_expired::execute() {
	Database db(GGG_DATABASE_PATH);
	auto rstr = db.expired_entities();
	user_iterator first(rstr), last;
	while (first != last) {
		this->_result.insert(*first);
		++first;
	}
	Show_base::execute();
}
