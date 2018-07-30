#include "show_members.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

#include <ggg/config.hh>
#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/ggg/quiet_error.hh>

#include "object_traits.hh"

void
ggg::Show_members::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Show_members::execute() {
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	const auto& h = g.hierarchy();
	int nerrors = 0;
	for (const std::string& a : this->args()) {
		auto it0 = h.find_group_by_name(a.data());
		if (it0 == h.group_end()) {
			++nerrors;
			native_message(std::cerr, "Unable to find _.", a);
		} else {
			for (const std::string& m : it0->members()) {
				auto it = h.find_by_name(m.data());
				if (it == h.end()) {
					++nerrors;
					native_message(std::cerr, "Unable to find _.", m);
				} else {
					this->_result.insert(*it);
				}
			}
		}
	}
	Show_base::execute();
	if (nerrors > 0) {
		throw quiet_error();
	}
}
