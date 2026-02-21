#include <gtest/gtest.h>
#include <unix.hpp>


TEST(UnixSimpleTests, SimpleCases)
{
    Solution solver;

    String s = solver.solve("/sssss////ss/s");

    EXPECT_STREQ(s.getConstData(), "/sssss/ss/s");

    s = solver.solve("/s/../s/./s/");

    EXPECT_STREQ(s.getConstData(), "/s/s");
}