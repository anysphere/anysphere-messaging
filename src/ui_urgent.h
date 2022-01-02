
#include <stdlib.h>

#include "common.h"

void process_ui_urgent_file(const string& ui_urgent_file_address,
                            const Time& last_ui_urgent_timestamp,
                            std::unique_ptr<Messenger::Stub>& stub) {
  auto new_entries =
      get_new_entries(ui_urgent_file_address, last_ui_urgent_timestamp);

  if (new_entries.empty()) {
    cout << "no new entries" << endl;
    return;
  }
  auto entry = new_entries.back();

  auto type = entry["type"];
  auto timestamp = entry["timestamp"];
  if (type == "register") {
    // call register rpc to send the register request
    RegisterInfo request;
    auto publickey = 543210;
    request.set_public_key(to_string(publickey));

    RegisterResponse reply;
    ClientContext context;

    Status status = stub->Register(&context, request, &reply);

    if (status.ok()) {
      // TODO(sualeh): tell the UI that the registration was successful
      cout << "register success" << endl;
    } else {
      cout << status.error_code() << ": " << status.error_message() << endl;
    }
  } else {
    std::cerr << "Unknown type of message. Ensure that messages are correctly "
                 "written to file. "
              << type << std::endl;
    exit(1);
  }
}