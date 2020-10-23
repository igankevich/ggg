#include <gtest/gtest.h>

#include <ggg/test/clean_database.hh>

TEST(passwd, getpwent) {
    {
        Clean_database db;
        db.insert("testuser:x:2000:2000:halt:/sbin:/sbin/halt");
    }
}
