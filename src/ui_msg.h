#include "common.h"

using messenger::SendMessageInfo;
using messenger::SendMessageResponse;

auto get_index() { return absl::Uniform(gen_, 0, 100); }
auto get_auth_token()
{
  return StrCat("ThisIsMyGreatAuthToken ", absl::Uniform(gen_, 0, 100));
}

void process_ui_file(const string &ui_file_address,
                     const Time &last_ui_timestamp,
                     std::unique_ptr<Messenger::Stub> &stub)
{
  cout << "here" << endl;
  auto new_entries = get_new_entries(ui_file_address, last_ui_timestamp);

  auto index = get_index();
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
    // BUG(sualeh): Fix this using ParseTime
    Time timestamp;
    string err;
    absl::ParseTime(absl::RFC3339_full, jt, &timestamp, &err);
    // Time timestamp = absl::FromUnixSeconds(std::stoull(jt));

    auto msg = entry["message"].get<string>();

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
          std::cout << status.error_code() << ": " << status.error_message()
                    << std::endl;
        }
        // } else if (type == "error") {
        //   // call register rpc to send the register request
        //   SendMessageInfo request;
        //   request.set_index(index);
        //   request.set_authentication_token(authentication_token);
        //   request.set_message(entry["message"]);

        //   SendMessageResponse reply;
        //   ClientContext context;

        //   Status status = stub->Register(&context, request, &reply);

        //   if (status.ok()) {
        //     std::cout << "Error sent to server: " << request.message() <<
        //     std::endl;
        //   } else {
        //     std::cout << status.error_code() << ": " << status.error_message()
        //               << std::endl;
        //   }
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
