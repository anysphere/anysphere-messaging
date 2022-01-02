#include <gtest/gtest.h>
#include "server/src/pir/fastpir/fastpir.h"
#include "server/src/pir/fastpir/fastpir_client.h"

#include <string>

using std::string;

TEST(FastPirTest, Basic)
{
    FastPIR pir;
    auto idx = pir.allocate();
    EXPECT_EQ(idx, 0);
    string value_str = "hello";
    pir_value_t value;
    std::copy(value_str.begin(), value_str.end(), value.begin());
    pir.set_value(idx, value);

    FastPIRClient client;
    auto query = client.query(idx);

    auto answer = pir.get_value_privately(query);

    auto decoded_value = client.decode(answer);

    EXPECT_EQ(decoded_value, value);
}