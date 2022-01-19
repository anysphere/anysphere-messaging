
#include "anysphere.hpp"

#include "ui_msg.hpp"

int main(int argc, char** argv) {
  std::string server_address(SERVER_ADDRESS);

  auto socket_address = string("");
  auto config_file_address = string("");

  vector<string> args(argv + 1, argv + argc);
  string infname, outfname;

  // Loop over command-line args
  for (auto i = args.begin(); i != args.end(); ++i) {
    if (*i == "-h" || *i == "--help") {
      std::cout << "Syntax: daemon -s <server_address> -d <socket_address>"
                   " -c <config_file_address>"
                << std::endl;
      std::cout << "  -s <server_address>  Address to listen on (default: "
                << server_address << ")" << std::endl;
      std::cout << "  -d <socket_address>  Address of socket (default: "
                << socket_address << ")" << std::endl;
      std::cout
          << "  -c <config_file_address>  Address of config file (default: "
          << config_file_address << ")" << std::endl;
      return 0;
    } else if (*i == "-s") {
      server_address = *++i;
    } else if (*i == "-d") {
      socket_address = *++i;
    } else if (*i == "-c") {
      config_file_address = *++i;
    }
  }

  if (socket_address == "") {
    socket_address = get_socket_path().string();
  }
  if (config_file_address == "") {
    config_file_address = get_config_file_address().string();
  }

  Config config(config_file_address);

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
  auto daemon = DaemonRpc(crypto, config, stub);
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

    cout << "send messages file address: " << config.send_file_address()
         << endl;
    process_ui_file(config.send_file_address(), last_ui_timestamp, stub, crypto,
                    config);
    last_ui_timestamp = absl::Now();
    cout << "received messages file address: " << config.receive_file_address()
         << endl;
    retrieve_messages(config.receive_file_address(), stub, crypto, config);

    // sleep for 100ms
  }

  return 0;
}
