
#include "messenger_impl.h"

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
//    -

// TODO: look into AsyncService; might be useful for performance
using index_type = uint64_t;
using value_type = std::string;
using pir_query_type = pir_query_type_base;
using pir_answer_type = pir_answer_type_base;

int main(int argc, char **argv) {
  std::string server_address("0.0.0.0:50051");
  if (argc > 1) {
    server_address = argv[1];
  }

  auto pir_db = make_shared<
      PirDB<index_type, value_type, pir_query_type, pir_answer_type>>();

  MessengerImpl<index_type, value_type, pir_answer_type, pir_query_type>
      messenger_service(pir_db);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&messenger_service);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
