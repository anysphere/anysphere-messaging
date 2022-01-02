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
    auto query = client.query(idx, 1);

    auto serialized_query = query.serialize_to_string();

    auto deserialized_query = pir.query_from_string(serialized_query);

    auto answer = pir.get_value_privately(deserialized_query);

    auto decoded_value = client.decode(answer);

    EXPECT_EQ(decoded_value, value);
}