#include <ggg/cli/show_duplicates.hh>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>

#include <unistdx/system/nss>

#include <ggg/config.hh>
#include <ggg/core/entity.hh>
#include <ggg/cli/quiet_error.hh>

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
    std::multiset<entity, compare_name> set2;
    sys::userstream users;
    std::for_each(
        sys::user_iterator(users),
        sys::user_iterator(),
        [&set1,&set2] (const sys::user& rhs) {
            entity tmp;
            tmp = rhs;
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
    if (!duplicates.empty()) {
        throw quiet_error();
    }
}

void
ggg::Show_duplicates::print_usage() {
    std::cout << "usage: " GGG_EXECUTABLE_NAME " "
              << this->prefix() << " COMMAND" << '\n';
}
