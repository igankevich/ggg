#include <vector>

#include <ggg/core/group.hh>

#include <gtest/gtest.h>

class GroupTest: public ::testing::TestWithParam<ggg::group> {};

TEST_P(GroupTest, BstreamIO) {
    ggg::group ent(GetParam()), ent2;
    std::stringbuf buf;
    sys::bstream str(&buf);
    str << ent;
    str >> ent2;
    EXPECT_EQ(ent, ent2);
}

INSTANTIATE_TEST_CASE_P(
    All,
    GroupTest,
    ::testing::Values(
        ggg::group(),
        ggg::group("g1"),
        ggg::group("g1", 1231),
        ggg::group("g1", 1231, {"a", "b", "c"}),
        ggg::group("g1", 1231, {"a", "b"}),
        ggg::group("g1", 1231, {"a"})
    )
);
