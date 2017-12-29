#include "show_members.hh"

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <iterator>
#include <locale>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <ctl/ggg.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Show_members::parse_arguments(int argc, char* argv[]) {
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
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Show_members::execute() {
	typedef Object_traits<entity> traits_type;
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	const Hierarchy& h = g.hierarchy();
	std::set<entity> result;
	for (const std::string& a : this->args()) {
		auto it0 = h.find_group_by_name(a.data());
		if (it0 == h.group_end()) {
			std::cerr << "Unable to find " << a << std::endl;
		} else {
			for (const std::string& m : it0->members()) {
				auto it = h.find_by_name(m.data());
				if (it == h.end()) {
					std::cerr << "Unable to find " << a << std::endl;
				} else {
					result.insert(*it);
				}
			}
		}
	}
	if (this->_long) {
		std::locale::global(std::locale(""));
		std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
		std::set<wentity> wresult;
		for (const entity& ent : result) {
			wresult.emplace(ent, cv);
		}
		align_columns(wresult, std::wcout, wentity::delimiter);
	} else {
		std::transform(
			result.begin(),
			result.end(),
			std::ostream_iterator<std::string>(std::cout, "\n"),
			[] (const entity& ent) {
				return ent.name();
			}
		);
	}
}

void
ggg::Show_members::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " [-l] ENTITY..." << '\n';
}

