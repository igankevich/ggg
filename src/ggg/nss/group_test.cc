#include <ggg/nss/entity_traits.hh>
#include <ggg/test/clean_database.hh>
#include <grp.h>
#include <gtest/gtest.h>

struct gr_guard {
    gr_guard() { ::setgrent(); }
    ~gr_guard() { ::endgrent(); }
};

TEST(group, empty) {
    { Clean_database db; }
    gr_guard g;
    struct ::group* ent = ::getgrent();
    ASSERT_EQ(nullptr, ent) << ent->gr_name;
}

TEST(group, getgrent) {
    {
        Clean_database db;
        db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
    }
    gr_guard g;
    struct ::group* ent = ::getgrent();
    ASSERT_NE(nullptr, ent);
    EXPECT_STREQ("testuser", ent->gr_name);
}

TEST(group, getgrnam) {
    {
        Clean_database db;
        db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
        db.insert("user2:x:2001:2001:halt:/sbin:/sbin/halt");
        db.insert("user3:x:2002:2002:halt:/sbin:/sbin/halt");
    }
    struct group* user1 = getgrnam("testuser");
    ASSERT_NE(nullptr, user1);
    EXPECT_STREQ("testuser", user1->gr_name);
    EXPECT_EQ(2000u, user1->gr_gid);
    struct group* user2 = getgrnam("user2");
    EXPECT_STREQ("user2", user2->gr_name);
    EXPECT_EQ(2001u, user2->gr_gid);
    ASSERT_NE(nullptr, user2);
    struct group* user3 = getgrnam("user3");
    ASSERT_NE(nullptr, user3);
    EXPECT_STREQ("user3", user3->gr_name);
    EXPECT_EQ(2002u, user3->gr_gid);
    struct group* user4 = getgrnam("nonexistent");
    ASSERT_EQ(nullptr, user4);
}

TEST(group, getgrgid) {
    {
        Clean_database db;
        db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
        db.insert("user2:x:2001:2001:halt:/sbin:/sbin/halt");
        db.insert("user3:x:2002:2002:halt:/sbin:/sbin/halt");
    }
    struct group* user1 = getgrgid(2000);
    ASSERT_NE(nullptr, user1);
    EXPECT_STREQ("testuser", user1->gr_name);
    EXPECT_EQ(2000u, user1->gr_gid);
    struct group* user2 = getgrgid(2001);
    EXPECT_STREQ("user2", user2->gr_name);
    EXPECT_EQ(2001u, user2->gr_gid);
    ASSERT_NE(nullptr, user2);
    struct group* user3 = getgrgid(2002);
    ASSERT_NE(nullptr, user3);
    EXPECT_STREQ("user3", user3->gr_name);
    EXPECT_EQ(2002u, user3->gr_gid);
    struct group* user4 = getgrgid(12345);
    ASSERT_EQ(nullptr, user4);
}

TEST(group, getgrouplist) {
    {
        Clean_database db;
        db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
    }
    int ngroups = 4096 / sizeof(::gid_t);
    ::gid_t groups[ngroups];
    int ret = ::getgrouplist("testuser", 2000, groups, &ngroups);
    ASSERT_EQ(1, ret);
}

TEST(group, group_member_via_tie) {
    {
        Clean_database db;
        db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
        db.insert("testgroup:x:2001:2001:halt:/sbin:/sbin/halt");
        db.tie("testuser", "testgroup", ggg::Database::Ties::User_group);
    }
    auto* testgroup = getgrnam("testgroup");
    EXPECT_STREQ("testuser", *testgroup->gr_mem);
    int ngroups = 4096 / sizeof(::gid_t);
    ::gid_t groups[ngroups];
    int ret = ::getgrouplist("testuser", 2000, groups, &ngroups);
    if (ret > 0 && ret != 2) {
        for (int i=0; i<ret; ++i) {
            std::clog << "groups[i]=" << groups[i] << std::endl;
        }
    }
    ASSERT_EQ(2, ret);
    if (groups[0] == 2000) {
        EXPECT_EQ(2001u, groups[1]);
    } else if (groups[0] == 2001) {
        EXPECT_EQ(2000u, groups[1]);
    } else {
        FAIL() << "groups=" << groups[0] << ',' << groups[1];
    }
}

TEST(group, getgrent_nested_groups) {
    {
        Clean_database db;
        db.insert("u1:x:2000:2000:halt:/sbin:/sbin/halt");
        db.insert("g1:x:2001:2001:halt:/sbin:/sbin/halt");
        db.insert("g2:x:2002:2002:halt:/sbin:/sbin/halt");
        db.tie("u1", "g2", ggg::Database::Ties::User_group);
        db.tie("g1", "g2", ggg::Database::Ties::Group_group);
    }
    auto* u1 = getgrnam("u1");
    ASSERT_NE(nullptr, u1->gr_mem);
    EXPECT_EQ(nullptr, u1->gr_mem[0]) << u1->gr_mem[0];
    auto* g1 = getgrnam("g1");
    ASSERT_NE(nullptr, g1->gr_mem);
    ASSERT_NE(nullptr, g1->gr_mem[0]) << g1->gr_mem[0];
    ASSERT_EQ(nullptr, g1->gr_mem[1]) << g1->gr_mem[1];
    EXPECT_STREQ("u1", g1->gr_mem[0]);
    auto* g2 = getgrnam("g2");
    ASSERT_NE(nullptr, g2->gr_mem);
    ASSERT_NE(nullptr, g2->gr_mem[0]) << g2->gr_mem[0];
    ASSERT_EQ(nullptr, g2->gr_mem[1]) << g2->gr_mem[1];
    EXPECT_STREQ("u1", g2->gr_mem[0]);
    gr_guard g;
    while (const auto* ent = ::getgrent()) {
        std::string name = ent->gr_name;
        ggg::group group(*ent);
        if (name == "u1") {
            EXPECT_EQ(ggg::group::container_type{}, group.members());
        } else if (name == "g1") {
            EXPECT_EQ(ggg::group::container_type{"u1"}, group.members());
        } else if (name == "g2") {
            EXPECT_EQ(ggg::group::container_type{"u1"}, group.members());
        } else {
            FAIL() << "unexpected group: " << name;
        }
    }
}

TEST(group, copy) {
    ggg::group ent;
    char fillchar = -1;
    size_t bs = buffer_size(ent);
    std::vector<char> buffer((bs * 2) | 64, fillchar);
    struct ::group gr;
    ggg::copy_to(ent, &gr, buffer.data());
    EXPECT_EQ(fillchar, buffer[bs]);
    EXPECT_STREQ(ent.name().data(), gr.gr_name);
    EXPECT_EQ(ent.id(), gr.gr_gid);
    EXPECT_STREQ("", gr.gr_passwd);
    std::unordered_set<std::string> mem;
    char** first = gr.gr_mem;
    while (*first) {
        mem.emplace(*first);
        ++first;
    }
    EXPECT_EQ(ent.members(), mem);
}
