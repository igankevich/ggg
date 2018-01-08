#include "show_groups.hh"

#include <iostream>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <core/native.hh>
#include <ctl/ggg.hh>

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
	WGGG g(GGG_ENT_ROOT, this->verbose());
	const auto& h = g.hierarchy();
	wentity::wcvt_type cv;
	for (const std::string& a : this->args()) {
		std::wstring wa = cv.from_bytes(a);
		auto it0 = h.find_by_name(wa.data());
		if (it0 == h.end()) {
			native_message(std::wcerr, "Unable to find _.\n", wa);
		} else {
			const wentity& ent = *it0;
			auto groups = h.find_supplementary_groups(ent.name());
			for (const std::wstring& g : groups) {
				auto it1 = h.find_by_name(g.data());
				if (it1 == h.end()) {
					native_message(std::wcerr, "Unable to find _.\n", g);
				} else {
					this->_result.insert(*it1);
				}
			}
		}
	}
	Show_base::execute();
}

