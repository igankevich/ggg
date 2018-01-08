#include "show_expired.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <core/native.hh>
#include <ctl/ggg.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Show_expired::parse_arguments(int argc, char* argv[]) {
	Show_base::parse_arguments(argc, argv);
	if (!this->_args.empty()) {
		throw std::invalid_argument("please, do not specify entity names");
	}
}

void
ggg::Show_expired::execute() {
	file_lock lock;
	wentity::wcvt_type cv;
	WGGG g(GGG_ENT_ROOT, this->verbose());
	const auto& h = g.hierarchy();
	const account_ctl& accounts = g.accounts();
	accounts.for_each([&] (const account& acc) {
		if (acc.has_expired()) {
			std::wstring wlogin = cv.from_bytes(acc.login().data());
			auto it = h.find_by_name(wlogin.data());
			if (it == h.end()) {
				native_message(std::wcerr, "Unable to find _.\n", wlogin);
			} else {
				this->_result.insert(*it);
			}
		}
	});
	Show_base::execute();
}
