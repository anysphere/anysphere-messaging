
#include "ui_urgent.h"
#include "ui_msg.h"


int main(int argc, char **argv) {
  std::string server_address("0.0.0.0:50051");
  if (argc > 1) {
    server_address = argv[1];
  }

  // file address of where the UI and client will communicate
  auto ui_write_file_address("/etc/anysphere/ui.jsonl");
  auto ui_urgent_file_address("/etc/anysphere/ui-urgent.jsonl");
  auto client_write_file_address("/etc/anysphere/client.jsonl");
  if (argc > 4) {
    ui_write_file_address = argv[2];
    ui_urgent_file_address = argv[3];
    client_write_file_address = argv[4];
  }

  // configuration file
  auto config_file_address("/etc/anysphere/config.json");
  if (argc > 5) {
    config_file_address = argv[4];
  }

  // connect to the anysphere servers
  // std::cout << "Client querying server address: " << server_address
  //           << std::endl;
  // std::shared_ptr<Channel> channel =
  //     grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  // std::unique_ptr<Messenger::Stub> stub = Messenger::NewStub(channel);
  
  // get the local time using the system clock
  auto start_t = std::chrono::system_clock().now();

  // get a duration using chrono
  auto duration = std::chrono::milliseconds(100);
  auto round_duration = std::chrono::milliseconds(1000);

  // set the time to 0
  auto last_ui_timestamp = std::chrono::system_clock::time_point::min();
  auto last_ui_urgent_timestamp = std::chrono::system_clock::time_point::min();

  write_msg_to_file(client_write_file_address, "client started");
  auto new_msg = get_new_entries(client_write_file_address, last_ui_timestamp);
  return 0; 


  while (true) {

    // check for new ui write:
    // read from ui_urgent_file_address
    // auto ui_urgent_file = std::ifstream(ui_urgent_file_address);
    // if (ui_urgent_file.is_open()) {
    //   std::string line;
    //   while (std::getline(ui_urgent_file, line)) {
    //     auto j = json::parse(line);
    //     auto 
    //   }
    //   ui_urgent_file.close();
    // }
    process_ui_urgent_file(ui_urgent_file_address, last_ui_urgent_timestamp);

    // get the time difference from t
    auto now = std::chrono::system_clock().now();
    auto time_since_last_round = now - start_t;

    // if 1s has passed, then do a round
    if (time_since_last_round > round_duration) {
      // do a round
      std::cout << "Client round" << std::endl;
      // get the local time using the system clock
      start_t = std::chrono::system_clock().now();

      process_ui_file(ui_write_file_address, last_ui_timestamp);
    }

    // sleep for 100ms
    std::this_thread::sleep_for(duration);
  }

  return 0;
}
