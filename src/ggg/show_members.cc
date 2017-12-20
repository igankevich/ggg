#include "show_members.hh"

#include <iostream>
#include <set>
#include <string>

#include <config.hh>
#include <core/lock.hh>
#include <ctl/ggg.hh>

#include "align_columns.hh"
#include "object_traits.hh"

void
ggg::Show_members::parse_arguments(int argc, char* argv[]) {
	Command::parse_arguments(argc, argv);
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
	align_columns(result, std::cout, traits_type::delimiter());
}

void
ggg::Show_members::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " COMMAND" << '\n';
}

