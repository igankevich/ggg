#include "show_members.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <core/native.hh>
#include <ctl/ggg.hh>

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
	WGGG g(GGG_ENT_ROOT, this->verbose());
	const auto& h = g.hierarchy();
	wentity::wcvt_type cv;
	for (const std::string& a : this->args()) {
		std::wstring wa = cv.from_bytes(a);
		auto it0 = h.find_group_by_name(wa.data());
		if (it0 == h.group_end()) {
			native_message(std::wcerr, "Unable to find _.\n", wa);
		} else {
			for (const std::wstring& m : it0->members()) {
				auto it = h.find_by_name(m.data());
				if (it == h.end()) {
					native_message(std::wcerr, "Unable to find _.\n", m);
				} else {
					this->_result.insert(*it);
				}
			}
		}
	}
	Show_base::execute();
}
