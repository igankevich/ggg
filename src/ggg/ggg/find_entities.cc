#include "find_entities.hh"

#include <codecvt>
#include <iostream>
#include <regex>
#include <set>
#include <string>

#include <ggg/config.hh>
#include <ggg/core/lock.hh>
#include <ggg/ctl/ggg.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Find_entities::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
}

void
ggg::Find_entities::execute() {
	file_lock lock;
	WGGG g(GGG_ENT_ROOT, this->verbose());
	const auto& h = g.hierarchy();
	if (this->_args.empty()) {
		std::copy(
			h.begin(),
			h.end(),
			std::inserter(this->_result, this->_result.begin())
		);
	} else {
		wentity::wcvt_type cv;
		for (const std::string& a : this->args()) {
			using namespace std::regex_constants;
			std::wregex wexpr(cv.from_bytes(a), ECMAScript | optimize | icase);
			std::copy_if(
				h.begin(),
				h.end(),
				std::inserter(this->_result, this->_result.begin()),
				[&] (const wentity& ent) {
				    return std::regex_search(ent.name(), wexpr) ||
				           std::regex_search(ent.real_name(), wexpr);
				}
			);
		}
	}
	Show_base::execute();
}

void
ggg::Find_entities::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " REGEX..." << '\n';
}
