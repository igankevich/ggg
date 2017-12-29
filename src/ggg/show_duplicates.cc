#include "show_duplicates.hh"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>

#include <unistdx/util/userstream>

#include <config.hh>
#include <core/entity.hh>

namespace {
	struct compare_uid {
		inline bool
		operator()(const ggg::entity& lhs, const ggg::entity& rhs) const {
			return lhs.id() < rhs.id();
		}

	};

	struct compare_name {
		inline bool
		operator()(const ggg::entity& lhs, const ggg::entity& rhs) const {
			return lhs.name() < rhs.name();
		}

	};
}

void
ggg::Show_duplicates::execute() {
	std::multiset<entity, compare_uid> set1;
	std::multiset<entity, compare_uid> set2;
	sys::userstream users;
	std::for_each(
		sys::user_iterator(users),
		sys::user_iterator(),
		[&set1,&set2] (const sys::user& rhs) {
			entity tmp;
			assign(tmp, rhs);
		    set1.insert(tmp);
		    set2.insert(tmp);
		}
	);
	std::set<entity> duplicates;
	for (entity ent : set1) {
		if (set1.count(ent) > 1) {
			duplicates.insert(ent);
		}
	}
	for (entity ent : set2) {
		if (set2.count(ent) > 1) {
			duplicates.insert(ent);
		}
	}
	for (entity ent : duplicates) {
		std::cout << ent << '\n';
	}
}

void
ggg::Show_duplicates::print_usage() {
	std::cout << "usage: " GGG_EXECUTABLE_NAME " "
	          << this->prefix() << " COMMAND" << '\n';
}
