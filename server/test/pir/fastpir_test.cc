#include "server/pir/fastpir/fastpir.h"

#include <gtest/gtest.h>

#include <string>

#include "server/pir/fastpir/fastpir_client.h"

using std::cout;
using std::endl;
using std::string;

TEST(FastPirTest, Basic) {
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

TEST(FastPirTest, Basic2) {
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

TEST(FastPirTest, Basic10) {
  FastPIR pir;
  for (int i = 0; i < 10; i++) {
    auto idx = pir.allocate();
    EXPECT_EQ(idx, i);
    std::stringstream ss;
    ss << "hello " << i;
    string value_str = ss.str();
    pir_value_t value;
    value.fill(0);
    std::copy(value_str.begin(), value_str.end(), value.begin());
    pir.set_value(idx, value);
  }

  for (int i = 0; i < 10; i++) {
    FastPIRClient client;

    auto query = client.query(i, 10);

    auto serialized_query = query.serialize_to_string();

    auto deserialized_query = pir.query_from_string(serialized_query);

    auto answer = pir.get_value_privately(deserialized_query);

    auto decoded_value = client.decode(answer, i);

    std::stringstream ss;
    ss << "hello " << i;
    string value_str = ss.str();
    pir_value_t expected_value;
    expected_value.fill(0);
    std::copy(value_str.begin(), value_str.end(), expected_value.begin());

    EXPECT_EQ(decoded_value, expected_value);
  }
}