
#include "anysphere.hpp"

#include "transmitter.hpp"

int main(int argc, char** argv) {
  auto server_address = string("");
  auto socket_address = string("");
  auto config_file_address = string("");
  auto round_delay = DEFAULT_ROUND_DELAY_SECONDS;
  auto tls = true;

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
      std::cout << "  -r <round_delay>  Round delay in seconds (default: "
                << round_delay << ")" << std::endl;
      std::cout << "  --no-tls  Don't use TLS (default: use tls)" << std::endl;
      return 0;
    } else if (*i == "-s") {
      server_address = *++i;
    } else if (*i == "-d") {
      socket_address = *++i;
    } else if (*i == "-c") {
      config_file_address = *++i;
    } else if (*i == "-r") {
      round_delay = std::stoi(*++i);
    } else if (*i == "--no-tls") {
      tls = false;
    } else {
      std::cerr << "Unknown argument: " << *i << std::endl;
      return 1;
    }
  }

  if (socket_address == "") {
    socket_address = get_socket_path().string();
  }
  if (config_file_address == "") {
    config_file_address = get_config_file_address().string();
  }

  auto config = make_shared<Config>(config_file_address);

  if (server_address == "") {
    server_address = config->server_address();
  }

  const Crypto crypto;

  // remove the socket file first
  remove(socket_address.c_str());

  // make it a socket address!
  socket_address = StrCat("unix://", socket_address);

  // connect to the anysphere servers
  cout << "Client querying server address: " << server_address << std::endl;
  auto channel_creds = grpc::SslCredentials(
      grpc::SslCredentialsOptions{.pem_root_certs = AMAZON_ROOT_CERTS});
  if (!tls) {
    channel_creds = grpc::InsecureChannelCredentials();
  }
  shared_ptr<grpc::Channel> channel =
      grpc::CreateChannel(server_address, channel_creds);
  shared_ptr<asphrserver::Server::Stub> stub =
      asphrserver::Server::NewStub(channel);

  // TODO: VERIFY AND MAKE SURE CRYPTO, CONFIG, STUB ARE ALL THREADSAFE!!!!!
  Transmitter transmitter(crypto, config, stub);

  // set up the daemon rpc server
  auto daemon = DaemonRpc(crypto, config, stub);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(socket_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&daemon);

  // start the daemon rpc server
  auto daemon_server = unique_ptr<grpc::Server>(builder.BuildAndStart());

  while (true) {
    auto killed = config->wait_until_killed_or_seconds(round_delay);
    if (killed) {
      daemon_server->Shutdown();
      cout << "Daemon killed!" << endl;
      break;
    }

    // do a round
    std::cout << "Client round" << std::endl;

    // receive and then send! it is important! 2x speedup
    transmitter.retrieve_messages();
    transmitter.send_messages();

    // sleep for 100ms
  }

  return 0;
}
