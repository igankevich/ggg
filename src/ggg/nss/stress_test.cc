#include <ggg/nss/entity_traits.hh>
#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>
#include <pwd.h>

struct pw_guard {
    pw_guard() { ::setpwent(); }
    ~pw_guard() { ::endpwent(); }
};

TEST(stress, fd_overlow) {
    {
        Clean_database db;
        for (int i=0; i<1024; ++i) {
            int id = 2000 + i;
            std::stringstream tmp;
            tmp << "testuser" << i << ":x:" << id << ':' << id
                << ":no description:/var/empty:/sbin/nologin";
            db.insert(tmp.str().data());
        }
    }
    pw_guard g;
    for (auto ent = ::getpwent(); ent; ent = ::getpwent()) {
        auto ent2 = getpwnam(ent->pw_name);
        ASSERT_NE(nullptr, ent2) << ent->pw_name;
        ggg::entity old_ent, new_ent;
        old_ent = *ent;
        new_ent = *ent2;
        EXPECT_EQ(old_ent.id(), new_ent.id());
        EXPECT_EQ(old_ent.name(), new_ent.name());
        EXPECT_EQ(old_ent.description(), new_ent.description());
        EXPECT_EQ(old_ent.shell(), new_ent.shell());
        EXPECT_EQ(old_ent.home(), new_ent.home());
    }
}
