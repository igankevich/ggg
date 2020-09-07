#include <ggg/test/clean_database.hh>
#include <gtest/gtest.h>

#include <thread>
#include <unistdx/ipc/process>

#define expect_no_throw(code) \
{ \
    try { \
        ggg::Transaction tr(db); \
        code; \
        tr.commit(); \
        SUCCEED(); \
    } \
    catch (const std::exception& err) { FAIL() << err.what(); } \
    catch (...) { FAIL() << "unknown error"; } \
}

#define expect_throw(code, exception_type) \
{ \
    try { \
        ggg::Transaction tr(db); \
        code; \
        tr.commit(); \
        FAIL() << "did not throw " #exception_type; \
    } \
    catch (const exception_type& err) { SUCCEED(); } \
    catch (const std::exception& err) { FAIL() << err.what(); } \
    catch (...) { FAIL() << "unknown error"; } \
}


/*
TEST(database, detect_loops) {
    // TODO does not work
    using namespace ggg;
    Clean_database db;
    db.insert(entity("u1"));
    db.insert(entity("u2"));
    db.insert(entity("u3"));
    db.insert(entity("users"));
    expect_no_throw(db.attach("u1", "users"));
    expect_no_throw(db.attach("u2", "users"));
    expect_no_throw(db.attach("u3", "users"));
    db.insert(entity("m1"));
    db.insert(entity("m2"));
    db.insert(entity("m3"));
    db.insert(entity("machines"));
    expect_no_throw(db.attach("machines", "m1"));
    expect_no_throw(db.attach("machines", "m2"));
    expect_no_throw(db.attach("machines", "m3"));
    // attach should not work for entities from different hierarchies
    expect_throw(db.attach("u1", "m1"), std::invalid_argument);
    expect_throw(db.attach("u1", "m2"), std::invalid_argument);
    expect_throw(db.attach("u1", "m3"), std::invalid_argument);
    expect_throw(db.attach("users", "machines"), std::invalid_argument);
    expect_throw(db.attach("machines", "users"), std::invalid_argument);
    expect_throw(db.attach("m1", "u1"), std::invalid_argument);
    expect_throw(db.attach("m2", "u2"), std::invalid_argument);
    expect_throw(db.attach("m3", "u3"), std::invalid_argument);
    // self loop
    expect_throw(db.attach("u1", "u1"), std::invalid_argument);
    expect_throw(db.attach("u2", "u2"), std::invalid_argument);
    expect_throw(db.attach("u3", "u3"), std::invalid_argument);
    expect_throw(db.attach("m1", "m1"), std::invalid_argument);
    expect_throw(db.attach("m2", "m2"), std::invalid_argument);
    expect_throw(db.attach("m3", "m3"), std::invalid_argument);
    // ties should not create loops
    expect_no_throw(db.tie("users", "machines"));
    expect_throw(db.tie("m1", "u1"), std::invalid_argument);
    // find hierarchy root should work in both directions
    {
        auto st = db.find_entity(db.find_hierarchy_root(db.find_id("u1")));
        EXPECT_NE(sqlite::errc::done, st.step());
        entity ent; st >> ent;
        EXPECT_EQ("users", ent.name());
    }
    {
        auto st = db.find_entity(db.find_hierarchy_root(db.find_id("m1")));
        EXPECT_NE(sqlite::errc::done, st.step());
        entity ent; st >> ent;
        EXPECT_EQ("machines", ent.name());
    }
}
*/

/*
TEST(database, locked) {
    using namespace ggg;
    Clean_database db;
    int nthreads = 20;
    std::vector<sys::process> threads;
    for (int i=0; i<nthreads; ++i) {
        threads.emplace_back([i,&db] () {
                std::printf("%lu\n", getpid());
            Database db(ggg::Database::File::All, ggg::Database::Flag::Read_write);
            for (int j=0; j<10; ++j) {
                std::string name;
                name += ('a'+i);
                name += std::to_string(j);
                try {
                    //Transaction tr(db);
                    db.message(name.data(), "authenticated");
                    //db.insert(entity(name));
                    //tr.commit();
                    std::printf("%s\n", name.data());
                } catch (const std::exception& err) {
                    std::printf("%s: %s\n", name.data(), err.what());
                }
            }
        });
    }
    for (auto& t : threads) { t.join(); }
}
*/
