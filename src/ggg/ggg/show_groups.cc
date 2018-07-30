#include "show_groups.hh"

#include <iostream>
#include <set>
#include <string>

#include <ggg/config.hh>
#include <ggg/core/lock.hh>
#include <ggg/core/native.hh>
#include <ggg/ctl/ggg.hh>
#include <ggg/ggg/quiet_error.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Show_groups::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
	if (this->_args.empty()) {
		throw std::invalid_argument("please, specify entity names");
	}
}

void
ggg::Show_groups::execute() {
	file_lock lock;
	GGG g(GGG_ENT_ROOT, this->verbose());
	const auto& h = g.hierarchy();
	int nerrors = 0;
	for (const std::string& a : this->args()) {
		auto it0 = h.find_by_name(a.data());
		if (it0 == h.end()) {
			++nerrors;
			native_message(std::cerr, "Unable to find _.", a);
		} else {
			const entity& ent = *it0;
			auto groups = h.find_supplementary_groups(ent.name());
			for (const std::string& g : groups) {
				auto it1 = h.find_by_name(g.data());
				if (it1 == h.end()) {
					++nerrors;
					native_message(std::cerr, "Unable to find _.", g);
				} else {
					this->_result.insert(*it1);
				}
			}
		}
	}
	Show_base::execute();
	if (nerrors > 0) {
		throw quiet_error();
	}
}

