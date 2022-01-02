#include <gtest/gtest.h>
#include "server/src/pir/fastpir/fastpir.h"

TEST(FastPirTest, Basic)
{
    FastPIR pir;
    auto idx = pir.allocate();
    EXPECT_EQ(idx, 0);
}