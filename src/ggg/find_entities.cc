#include "find_entities.hh"

#include <codecvt>
#include <iostream>
#include <regex>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <ctl/ggg.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Find_entities::parse_arguments(int argc, char* argv[]) {
	Command::parse_arguments(argc, argv);
}

void
ggg::Find_entities::execute() {
	typedef Object_traits<entity> traits_type;
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	std::locale::global(std::locale(""));
	const Hierarchy& h = g.hierarchy();
	std::set<entity> result;
	if (this->_args.empty()) {
		std::copy(
			h.begin(),
			h.end(),
			std::inserter(result, result.begin())
		);
	} else {
		std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
		for (const std::string& a : this->args()) {
			using namespace std::regex_constants;
			std::regex expr(a, ECMAScript | optimize | icase);
			std::wregex wexpr(cv.from_bytes(a), ECMAScript | optimize | icase);
			std::for_each(
				h.begin(),
				h.end(),
				[&] (const entity& ent) {
					std::wstring realname(cv.from_bytes(ent.real_name()));
				    if (std::regex_match(ent.name(), expr) ||
				        std::regex_match(realname, wexpr)) {
				        result.insert(ent);
					}
				}
			);
		}
	}
	align_columns(result, std::cout, traits_type::delimiter());
}

void
ggg::Find_entities::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " COMMAND" << '\n';
}
