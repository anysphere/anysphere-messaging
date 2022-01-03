#include "common.h"
#include "server/src/pir/fastpir/fastpir_client.h"

using messenger::ReceiveMessageInfo;
using messenger::ReceiveMessageResponse;
using messenger::SendMessageInfo;
using messenger::SendMessageResponse;

auto get_auth_token()
{
  return StrCat("ThisIsMyGreatAuthToken ", absl::Uniform(gen_, 0, 100));
}

void retrieve_messages(FastPIRClient &client,
                       std::unique_ptr<Messenger::Stub> &stub,
                       int db_rows)
{
  for (auto &friend_name : FriendTable)
  {
    ReceiveMessageInfo request;

    auto friend_info = FriendTable[friend_name];

    auto query = client.query(friend_info.read_index, db_rows);

    auto serialized_query = query.serialize_to_string();

    request.set_pir_query(serialized_query);

    ReceiveMessageResponse reply;
    ClientContext context;

    Status status = stub->ReceiveMessage(&context, request, &reply);

    if (status.ok())
    {
      cout << "received message!!!" << endl;
      auto answer = reply.pir_answer();
      auto answer_obj = client.answer_from_string(answer);
      auto decoded_value = client.decode(answer_obj, friend_info.read_index);

      string decoded_string = "";
      for (auto &c : decoded_value)
      {
        decoded_string += c;
      }

      cout << "actual message: " << decoded_string << endl;
    }
    else
    {
      cout << status.error_code() << ": " << status.error_message() << endl;
    }
  }
}

void process_ui_file(const string &ui_file_address,
                     const Time &last_ui_timestamp,
                     std::unique_ptr<Messenger::Stub> &stub)
{
  cout << "here" << endl;
  auto new_entries = get_new_entries(ui_file_address, last_ui_timestamp);

  auto authentication_token = get_auth_token();

  if (new_entries.empty())
  {
    return;
  }

  for (auto &entry : new_entries)
  {
    cout << "entry: " << entry << endl;
  }

  for (auto &entry : new_entries)
  {
    auto type = entry["type"].get<string>();
    auto jt = entry["timestamp"].get<string>();
    auto to = entry["to"].get<string>();
    // BUG(sualeh): Fix this using ParseTime
    Time timestamp;
    string err;
    absl::ParseTime(absl::RFC3339_full, jt, &timestamp, &err);
    // Time timestamp = absl::FromUnixSeconds(std::stoull(jt));

    auto msg = entry["message"].get<string>();

    if (!FriendHashTable.contains(to))
    {
      std::cerr << "FriendHashTable does not contain " << to << endl;
      continue;
    }

    auto friend = FriendHashTable[to];

    auto index = friend.write_index;
    auto key = friend.shared_key;

    cout << "Sending message to server: " << endl;
    cout << "type: " << type << endl;
    cout << "index: " << index << endl;
    cout << "authentication_token: " << authentication_token << endl;
    cout << "message: " << msg << endl;

    // static auto last_type = type;
    // static auto last_timestamp = timestamp;
    // static auto last_message = entry["message"];

    if (type == "MESSAGE")
    {
      cout << "timestamp: " << timestamp << endl;
      cout << "last_ui_timestamp: " << last_ui_timestamp << endl;
      if (timestamp > last_ui_timestamp)
      {
        // call register rpc to send the register request
        SendMessageInfo request;

        request.set_index(index);
        request.set_authentication_token(authentication_token);
        request.set_message(msg);

        SendMessageResponse reply;

        ClientContext context;

        Status status = stub->SendMessage(&context, request, &reply);

        if (status.ok())
        {
          std::cout << "Message sent to server: " << request.message()
                    << std::endl;
        }
        else
        {
          std::cerr << status.error_code() << ": " << status.error_message()
                    << " details:" << status.error_details() << std::endl;
        }
      }
    }
    else
    {
      std::cerr
          << "Unknown type of message. Ensure that messages are correctly "
             "written to file. "
          << type << std::endl;
      exit(1);
    }
  }
}
