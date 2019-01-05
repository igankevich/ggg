#include <algorithm>
#include <iostream>

#include <ggg/cli/copy.hh>
#include <ggg/cli/quiet_error.hh>
#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/hierarchy.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/account_ctl.hh>

void
ggg::Copy::execute() {
	Hierarchy h(GGG_ENT_ROOT);
	account_ctl accounts;
	Database db(Database::File::All, Database::Flag::Read_write);
	int nerrors = 0;
	for (const auto& ent : h) {
		try {
			db.insert(ent);
		} catch (const std::exception& err) {
			++nerrors;
			native_sentence(std::cerr, "Error adding _. ", ent);
			error_message(std::cerr, err);
		}
	}
	for (const auto& tie : h.ties()) {
		db.tie(tie.first, tie.second);
	}
	for (const auto& acc : accounts) {
		try {
			db.insert(acc);
		} catch (const std::exception& err) {
			++nerrors;
			native_sentence(std::cerr, "Error adding _. ", acc);
			error_message(std::cerr, err);
		}
	}
	if (nerrors > 0) {
		throw quiet_error();
	}
//	group gr;
//	if (db.find_group("spc2018", gr)) {
//	if (db.find_group(4256, gr)) {
//		std::cout << gr << std::endl;
//	}
//	auto groups = db.groups();
//	for (const auto& entry : groups) {
//		std::cout << entry.second << '\n';
//	}
//	auto rstr = db.accounts();
//	sqlite::rstream_iterator<ggg::account> first(rstr), last;
//	while (first != last) {
//		std::cout << *first++ << '\n';
//	}
//	db.dot(std::cout);
}

void
ggg::Copy::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << '\n';
}
