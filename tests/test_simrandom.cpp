#include <gtest/gtest.h>
#include "SimRandom.h"

TEST(SimRandomTests, Random01Range)
{
    for (int i = 0; i < 10000; i++) {
        double r = SimRandom::random01();
        EXPECT_GE(r, 0.0);
        EXPECT_LE(r, 1.0);
    }
}
