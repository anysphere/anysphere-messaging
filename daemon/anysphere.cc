
#include "anysphere.h"

#include "ui_msg.h"
#include "ui_urgent.h"

int main(int argc, char **argv) {
  std::string server_address("0.0.0.0:50051");
  if (argc > 1) {
    server_address = argv[1];
  }

  // file address of where the UI and client will communicate
  auto ui_write_file_address(UI_FILE);
  auto ui_urgent_file_address(UI_URGENT_FILE);
  auto client_write_file_address(CLIENT_FILE);
  auto config_file_address(CONFIG_FILE);
  if (argc > 5) {
    ui_write_file_address = argv[2];
    ui_urgent_file_address = argv[3];
    client_write_file_address = argv[4];
    config_file_address = argv[5];
  }

  read_config(config_file_address);

  // connect to the anysphere servers
  std::cout << "Client querying server address: " << server_address
            << std::endl;
  std::shared_ptr<Channel> channel =
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  std::unique_ptr<Messenger::Stub> stub = Messenger::NewStub(channel);

  // get the local time using the system clock
  auto start_t = absl::Now();

  // keep the duration in chrono for thread sleeping.
  constexpr auto duration = std::chrono::milliseconds(1000);
  constexpr auto round_duration = absl::Milliseconds(5000);

  // set the time to 0
  auto last_ui_timestamp = absl::Time();
  auto last_ui_urgent_timestamp = absl::Time();
  auto last_config_timestamp = absl::Time();

  write_msg_to_file(ui_write_file_address,
                    StrCat("Anysphere is worth ",
                           absl::Uniform(gen_, 100, 1000000000), " dollars"),
                    "MESSAGE", "sualeh");

  while (true) {
    // check for new ui write:

    process_ui_urgent_file(ui_urgent_file_address, config_file_address,
                           last_ui_urgent_timestamp, stub);

    last_ui_urgent_timestamp = absl::Now();
    last_config_timestamp = absl::Now();

    // get the time difference from t
    auto now = absl::Now();
    auto time_since_last_round = now - start_t;

    // if 1s has passed, then do a round
    if (time_since_last_round > round_duration) {
      // do a round
      std::cout << "Client round" << std::endl;
      start_t = absl::Now();  // reset the start time

      cout << "UI file address: " << ui_write_file_address << endl;
      process_ui_file(ui_write_file_address, last_ui_timestamp, stub);
      last_ui_timestamp = absl::Now();
      // first send, then retrieve, so we have an updated db_rows
      retrieve_messages(client_write_file_address, stub);
    }

    // sleep for 100ms

    std::this_thread::sleep_for(duration);
  }

  return 0;
}
