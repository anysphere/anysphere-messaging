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

    // cout << "query[0]: " << client.decrypt(query.query[0]).to_string() << endl;

    auto serialized_query = query.serialize_to_string();

    auto deserialized_query = pir.query_from_string(serialized_query);

    // cout << "deserialized_query[0]: " << client.decrypt(deserialized_query.query[0]).to_string() << endl;
    cout << "query len: " << deserialized_query.query.size() << endl;
    vector<uint64_t> query_vec;
    seal::BatchEncoder(client.get_context()).decode(client.decrypt(deserialized_query.query[0]), query_vec);
    cout << "deserialized_query: ";
    for (const auto &c : query_vec)
    {
        cout << static_cast<int>(c) << ",";
    }
    cout << endl;

    auto answer = pir.get_value_privately(deserialized_query, seal::Decryptor(client.get_context(), client.get_secret_key()));

    vector<uint64_t> answer_vec;
    seal::BatchEncoder(client.get_context()).decode(client.decrypt(answer.answer), answer_vec);
    cout << "answer: ";
    for (const auto &c : answer_vec)
    {
        cout << static_cast<int>(c) << ",";
    }
    cout << endl;

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

    // cout << "query[0]: " << client.decrypt(query.query[0]).to_string() << endl;

    auto serialized_query = query.serialize_to_string();

    auto deserialized_query = pir.query_from_string(serialized_query);

    // cout << "deserialized_query[0]: " << client.decrypt(deserialized_query.query[0]).to_string() << endl;
    cout << "query len: " << deserialized_query.query.size() << endl;
    vector<uint64_t> query_vec;
    seal::BatchEncoder(client.get_context()).decode(client.decrypt(deserialized_query.query[0]), query_vec);
    cout << "deserialized_query: ";
    for (const auto &c : query_vec)
    {
        cout << static_cast<int>(c) << ",";
    }
    cout << endl;

    auto answer = pir.get_value_privately(deserialized_query, seal::Decryptor(client.get_context(), client.get_secret_key()));

    vector<uint64_t> answer_vec;
    seal::BatchEncoder(client.get_context()).decode(client.decrypt(answer.answer), answer_vec);
    cout << "answer: ";
    for (const auto &c : answer_vec)
    {
        cout << static_cast<int>(c) << ",";
    }
    cout << endl;

    auto decoded_value = client.decode(answer, retrieve_idx);

    EXPECT_EQ(decoded_value, expected_value);
}