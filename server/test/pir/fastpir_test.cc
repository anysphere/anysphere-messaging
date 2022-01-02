#include <gtest/gtest.h>
#include "server/src/pir/fastpir/fastpir.h"

TEST(FastPirTest, Basic)
{
    FastPIR pir;
    auto idx = pir.allocate();
    EXPECT_EQ(idx, 0);
    auto value_str = "hello";
    pir_value_t value;
    std::copy(value_str.begin(), value_str.end(), value.begin());
    pir.set_value(idx, value);

}