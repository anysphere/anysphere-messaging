
#include "anysphere.hpp"

#include "ui_msg.hpp"

int main(int argc, char** argv) {
  std::string server_address("0.0.0.0:50051");

  std::string socket_address = DEFAULT_SOCKET_ADDRESS;

  auto ephemeralConfig = EphemeralConfig{
      .config_file_address = string(CONFIG_FILE),
      .send_messages_file_address = string(SEND_FILE),
      .received_messages_file_address = string(RECEIVE_FILE),
  };

  vector<string> args(argv + 1, argv + argc);
  string infname, outfname;

  // Loop over command-line args
  for (auto i = args.begin(); i != args.end(); ++i) {
    if (*i == "-h" || *i == "--help") {
      std::cout << "Syntax: daemon -a <server_address> -d <socket_address> -s "
                   "<send_messages_file_address> -r "
                   "<received_messages_file_address> -c <config_file_address>"
                << std::endl;
      std::cout << "  -a <server_address>  Address to listen on (default: "
                << server_address << ")" << std::endl;
      std::cout << "  -d <socket_address>  Address of socket (default: "
                << socket_address << ")" << std::endl;
      std::cout << "  -s <send_messages_file_address>  Address of send "
                   "messages file (default: "
                << ephemeralConfig.send_messages_file_address << ")"
                << std::endl;
      std::cout << "  -r <received_messages_file_address>  Address of received "
                   "messages file (default: "
                << ephemeralConfig.received_messages_file_address << ")"
                << std::endl;
      std::cout
          << "  -c <config_file_address>  Address of config file (default: "
          << ephemeralConfig.config_file_address << ")" << std::endl;
      return 0;
    } else if (*i == "-a") {
      server_address = *++i;
    } else if (*i == "-d") {
      socket_address = *++i;
    } else if (*i == "-s") {
      ephemeralConfig.send_messages_file_address = *++i;
    } else if (*i == "-r") {
      ephemeralConfig.received_messages_file_address = *++i;
    } else if (*i == "-c") {
      ephemeralConfig.config_file_address = *++i;
    }
  }

  Config config(ephemeralConfig.config_file_address);

  Crypto crypto;

  // connect to the anysphere servers
  cout << "Client querying server address: " << server_address << std::endl;
  shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  unique_ptr<asphrserver::Server::Stub> stub =
      asphrserver::Server::NewStub(channel);

  // keep the duration in chrono for thread sleeping.
  constexpr auto duration = absl::Milliseconds(5000);

  // set the time to 0
  auto last_ui_timestamp = absl::Time();

  // set up the daemon rpc server
  auto daemon = DaemonRpc(crypto, config, stub, ephemeralConfig);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(socket_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&daemon);

  // start the daemon rpc server
  auto daemon_server = unique_ptr<grpc::Server>(builder.BuildAndStart());

  while (true) {
    absl::SleepFor(duration);
    // check for new ui write:

    // do a round
    std::cout << "Client round" << std::endl;

    cout << "send messages file address: "
         << ephemeralConfig.send_messages_file_address << endl;
    process_ui_file(ephemeralConfig.send_messages_file_address,
                    last_ui_timestamp, stub, crypto, config);
    last_ui_timestamp = absl::Now();
    cout << "received messages file address: "
         << ephemeralConfig.received_messages_file_address << endl;
    retrieve_messages(ephemeralConfig.received_messages_file_address, stub,
                      crypto, config);

    // sleep for 100ms
  }

  return 0;
}
