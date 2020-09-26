#include <ggg/nss/entity_traits.hh>
#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>
#include <pwd.h>

struct pw_guard {
    pw_guard() { ::setpwent(); }
    ~pw_guard() { ::endpwent(); }
};

TEST(stress, users_1000) {
    {
        Clean_database db;
        for (int i=0; i<1000; ++i) {
            int id = 2000 + i;
            std::stringstream tmp;
            tmp << "testuser" << i << ":x:" << id << ':' << id << ":no description:/var/empty:/sbin/nologin";
            db.insert(tmp.str().data());
        }
    }
    /*
    pw_guard g;
    for (auto ent = ::getpwent(); ent; ent = ::getpwent()) {
        auto new_ent = getpwnam(ent->pw_name);
        ASSERT_NE(nullptr, new_ent) << ent->pw_name;
        ggg::entity tmp;
        tmp = *new_ent;
        std::clog << tmp << std::endl;
    }
    */
}
