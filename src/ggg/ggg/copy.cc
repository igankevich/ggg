#include <algorithm>
#include <iostream>

#include <ggg/config.hh>
#include <ggg/core/database.hh>
#include <ggg/core/hierarchy.hh>
#include <ggg/ctl/account_ctl.hh>
#include <ggg/ggg/copy.hh>

void
ggg::Copy::execute() {
	Hierarchy h(GGG_ENT_ROOT);
	account_ctl accounts;
	Database db(GGG_DATABASE_PATH, false);
	for (const auto& ent : h) {
		db.insert(ent);
	}
	for (const auto& tie : h.ties()) {
		db.tie(tie.first, tie.second);
	}
	for (const auto& acc : accounts) {
		try {
			db.update(acc);
		} catch (const std::exception& err) {
			std::clog << "bad account: " << acc << std::endl;
		}
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
