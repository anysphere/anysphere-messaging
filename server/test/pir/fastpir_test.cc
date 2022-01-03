#include <gtest/gtest.h>
#include "server/src/pir/fastpir/fastpir.h"
#include "server/src/pir/fastpir/fastpir_client.h"

#include <string>

using std::cout;
using std::endl;
using std::string;

TEST(FastPirTest, Basic)
{
    FastPIR pir;
    auto idx = pir.allocate();
    EXPECT_EQ(idx, 0);
    string value_str = "hello";
    pir_value_t value;
    value.fill(0);
    std::copy(value_str.begin(), value_str.end(), value.begin());
    pir.set_value(idx, value);

    auto idx2 = pir.allocate();
    EXPECT_EQ(idx2, 1);
    pir_value_t value2;
    value2.fill(0);
    pir.set_value(idx2, value2);

    auto retrieve_idx = idx2;
    auto expected_value = value2;

    FastPIRClient client;
    auto query = client.query(retrieve_idx, 2);

    auto serialized_query = query.serialize_to_string();

    auto deserialized_query = pir.query_from_string(serialized_query);

    auto answer = pir.get_value_privately(deserialized_query);

    auto decoded_value = client.decode(answer, retrieve_idx);

    EXPECT_EQ(decoded_value, expected_value);
}

TEST(FastPirTest, Basic2)
{
    FastPIR pir;
    auto idx = pir.allocate();
    EXPECT_EQ(idx, 0);
    string value_str = "hello";
    pir_value_t value;
    value.fill(0);
    std::copy(value_str.begin(), value_str.end(), value.begin());
    pir.set_value(idx, value);

    auto idx2 = pir.allocate();
    EXPECT_EQ(idx2, 1);
    pir_value_t value2;
    value2.fill(0);
    pir.set_value(idx2, value2);

    auto retrieve_idx = idx;
    auto expected_value = value;

    FastPIRClient client;
    auto query = client.query(retrieve_idx, 2);

    auto serialized_query = query.serialize_to_string();

    auto deserialized_query = pir.query_from_string(serialized_query);

    auto answer = pir.get_value_privately(deserialized_query);

    auto decoded_value = client.decode(answer, retrieve_idx);

    EXPECT_EQ(decoded_value, expected_value);
}