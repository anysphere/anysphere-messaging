
#include "messenger_impl.h"

#include "pir/nonprivate/nonprivate_pir.h"
#include "pir/sealpir/seal_test.h"
#include "account_manager.h"

// TODO(sualeh): turn on clang-tidy with bazel

// draft of structure:
//    - there are two kinds of databases. one contains all the messages and only
//    the messages, and is the one
//      we do PIR on. the other contains auxiliary non-private information, like
//      (authentication_token -> allocation) map, and possibly other things. we
//      care a lot about the latter database to be persistent, whereas the
//      former need not be persistent. so probably what we want is a postgres
//      instance for storing all non-private info, and an in-memory / custom
//      solution to performing the PIR (when memory usage becomes a problem,
//      split the PIR into multiple shards, and perform the queries on each
//      shard)

// TODO: look into AsyncService; might be useful for performance

int main(int argc, char **argv)
{
  std::string server_address("0.0.0.0:50051");
  std::string db_address("127.0.0.1");

  vector<string> args(argv + 1, argv + argc);
  string infname, outfname;

  // Loop over command-line args
  for (auto i = args.begin(); i != args.end(); ++i)
  {
    if (*i == "-h" || *i == "--help")
    {
      std::cout << "Syntax: as_server -a <server_address> -d <db_address> -m <in_memory_db>" << std::endl;
      std::cout << "  -a <server_address>  Address to listen on (default: " << server_address << ")" << std::endl;
      std::cout << "  -d <db_address>      Address of database (default: " << db_address << ")" << std::endl;
      return 0;
    }
    else if (*i == "-a")
    {
      server_address = *++i;
    }
    else if (*i == "-d")
    {
      db_address = *++i;
    }
  }

  // NonPrivatePIR pir;
  SealPIR pir;
#ifdef USE_MEMORY_DB
  using AccountManager = AccountManagerInMemory;
#else
  using AccountManager = AccountManagerPostgres;
#endif
  AccountManager account_manager;

  // MessengerImpl<NonPrivatePIR, AccountManager> messenger_service(pir, account_manager);
  MessengerImpl<SealPIR, AccountManager> messenger_service(pir, account_manager);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&messenger_service);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
