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
    std::copy(value_str.begin(), value_str.end(), value.begin());
    pir.set_value(idx, value);

    FastPIRClient client;
    auto query = client.query(idx, 1);

    // cout << "query[0]: " << client.decrypt(query.query[0]).to_string() << endl;

    auto serialized_query = query.serialize_to_string();

    auto deserialized_query = pir.query_from_string(serialized_query);

    cout << "deserialized_query[0]: " << client.decrypt(deserialized_query.query[0]).to_string() << endl;

    auto answer = pir.get_value_privately(deserialized_query, seal::Decryptor(client.get_context(), client.get_secret_key()));

    cout << "answer: " << client.decrypt(answer.answer).to_string() << endl;

    auto decoded_value = client.decode(answer, idx);

    EXPECT_EQ(decoded_value, value);
}