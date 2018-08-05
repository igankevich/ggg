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
	GGG::hierarchy_type h(GGG_ENT_ROOT, std::getenv("GGG_VERBOSE"));
	h.verbose(this->verbose());
	if (this->_args.empty()) {
		std::copy(
			h.begin(),
			h.end(),
			std::inserter(this->_result, this->_result.begin())
		);
	} else {
		bits::wcvt_type cv;
		for (const std::string& a : this->args()) {
			using namespace std::regex_constants;
			std::wregex wexpr(cv.from_bytes(a), ECMAScript | optimize | icase);
			std::copy_if(
				h.begin(),
				h.end(),
				std::inserter(this->_result, this->_result.begin()),
				[&] (const entity& ent) {
					std::wstring name = cv.from_bytes(ent.name());
					std::wstring real_name = cv.from_bytes(ent.real_name());
				    return std::regex_search(name, wexpr) ||
				           std::regex_search(real_name, wexpr);
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
