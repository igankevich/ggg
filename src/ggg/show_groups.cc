#include "show_groups.hh"

#include <iostream>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <ctl/ggg.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Show_groups::parse_arguments(int argc, char* argv[]) {
	Command::parse_arguments(argc, argv);
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Show_groups::execute() {
	typedef Object_traits<entity> traits_type;
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	const Hierarchy& h = g.hierarchy();
	std::set<entity> result;
	for (const std::string& a : this->args()) {
		auto it0 = h.find_by_name(a.data());
		if (it0 == h.end()) {
			std::cerr << "Unable to find " << a << std::endl;
		} else {
			const entity& ent = *it0;
			auto groups = h.find_supplementary_groups(ent.name());
			for (const std::string& g : groups) {
				auto it1 = h.find_by_name(g.data());
				if (it1 == h.end()) {
					std::cerr << "Unable to find " << g << std::endl;
				} else {
					result.insert(*it1);
				}
			}
		}
	}
	std::locale::global(std::locale(""));
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
	std::set<wentity> wresult;
	for (const entity& ent : result) {
		wresult.emplace(ent, cv);
	}
	align_columns(wresult, std::wcout, wentity::delimiter);
}

void
ggg::Show_groups::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " ENTITY" << '\n';
}

