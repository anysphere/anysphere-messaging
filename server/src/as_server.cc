
#include "account_manager.hpp"
#include "server/pir/fast_pir/fastpir.hpp"
#include "server/pir/nonprivate/nonprivate_pir.hpp"
#include "server_rpc.hpp"

// TODO: look into AsyncService; might be useful for performance

int main(int argc, char** argv) {
  std::string server_address("0.0.0.0:50051");
  std::string db_address("127.0.0.1");

  vector<string> args(argv + 1, argv + argc);
  string infname, outfname;

  // Loop over command-line args
  for (auto i = args.begin(); i != args.end(); ++i) {
    if (*i == "-h" || *i == "--help") {
      std::cout << "Syntax: as_server -a <server_address> -d <db_address> -m "
                   "<in_memory_db>"
                << std::endl;
      std::cout << "  -a <server_address>  Address to listen on (default: "
                << server_address << ")" << std::endl;
      std::cout << "  -d <db_address>      Address of database (default: "
                << db_address << ")" << std::endl;
      return 0;
    } else if (*i == "-a") {
      server_address = *++i;
    } else if (*i == "-d") {
      db_address = *++i;
    }
  }

  // NonPrivatePIR pir;
  // SealPIR pir;
  FastPIR pir;
  FastPIR pir_acks;
#ifdef USE_MEMORY_DB
  using AccountManager = AccountManagerInMemory;
#else
  using AccountManager = AccountManagerPostgres;
#endif
  AccountManager account_manager(db_address);

  // MessengerImpl<NonPrivatePIR, AccountManager> messenger_service(pir,
  // account_manager); MessengerImpl<SealPIR, AccountManager>
  // messenger_service(pir, account_manager);
  auto server_rpc = ServerRpc<FastPIR, AccountManager>(
      std::move(pir), std::move(pir_acks), std::move(account_manager));

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&server_rpc);
  auto server = unique_ptr<grpc::Server>(builder.BuildAndStart());

  cout << "Server listening on " << server_address << endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
