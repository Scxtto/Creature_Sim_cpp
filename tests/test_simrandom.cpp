#include <gtest/gtest.h>
#include "SimRandom.h"

TEST(SimRandomTests, urandRange)
{
    for (int i = 0; i < 10000; i++) {
        double r = SimRandom::urand();
        EXPECT_GE(r, 0.0);
        EXPECT_LE(r, 1.0);
    }
}
