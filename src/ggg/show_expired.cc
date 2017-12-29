#include "show_expired.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <ctl/ggg.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Show_expired::parse_arguments(int argc, char* argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "ql")) != -1) {
		switch (opt) {
			case 'q': this->_verbose = false; break;
			case 'l': this->_long = true; break;
		}
	}
	for (int i=::optind; i<argc; ++i) {
		this->_args.emplace(argv[i]);
	}
	if (!this->_args.empty()) {
		throw std::invalid_argument("please, do not specify entity names");
	}
}

void
ggg::Show_expired::execute() {
	typedef Object_traits<entity> traits_type;
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	const Hierarchy& h = g.hierarchy();
	const account_ctl& accounts = g.accounts();
	std::set<entity> result;
	accounts.for_each([&] (const account& acc) {
		if (acc.has_expired()) {
			auto it = h.find_by_name(acc.login().data());
			if (it == h.end()) {
				std::cerr << "Unable to find " << acc.login() << std::endl;
			} else {
				result.insert(*it);
			}
		}
	});
	if (this->_long) {
		std::locale::global(std::locale(""));
		std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
		std::set<wentity> wresult;
		for (const entity& ent : result) {
			wresult.emplace(ent, cv);
		}
		align_columns(wresult, std::wcout, wentity::delimiter);
	} else {
		for (const entity& ent : result) {
			std::cout << ent.name() << '\n';
		}
	}
}

void
ggg::Show_expired::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-l] ENTITY..." << '\n';
}

