#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#include "pir_db.h"

#ifdef BAZEL_BUILD
#include "schema/messenger.grpc.pb.h"
#else
#include "schema/messenger.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using messenger::Messenger;

using std::make_shared;
using std::shared_ptr;
using std::string;

template <typename index_type, typename value_type, typename pir_answer_type,
          typename pir_query_type>
class MessengerImpl final : public Messenger::Service {
  using PirDB = PirDB<index_type, value_type>;
  // TODO: add a thread safety argument (because the methods may be called from
  // different threads)
  // TODO: add representation invariant
  Status Register(ServerContext *context,
                  const messenger::RegisterInfo *registerInfo,
                  messenger::RegisterResponse *registerResponse) override {
    std::cout << "world" << std::endl;

    // return empty Status
    return Status::OK;
  }

  Status SendMessage(
      ServerContext *context, const messenger::SendMessageInfo *sendMessageInfo,
      messenger::SendMessageResponse *sendMessageResponse) override {
    // check that the authentication token corresponds to the index
    // check that the message is not too long
    std::cout << "world" << std::endl;

    // return empty Status
    return Status::OK;
  }

  Status ReceiveMessage(
      ServerContext *context,
      const messenger::ReceiveMessageInfo *receiveMessageInfo,
      messenger::ReceiveMessageResponse *receiveMessageResponse) override {
    auto input_query = receiveMessageInfo->pir_query();
    pir_query_type query;
    bool success = query.deserialize_from_string(input_query);
    if (!success) {
      std::cout << "error deserializing query" << std::endl;
      return Status::CANCELLED;
    }

    pir_answer_type answer = pir_db->get_value_privately(query);

    // serialize pir_answer_type
    string answer_string = answer.serialize_to_string();

    // TODO: use set_pir_answer or set_allocated_pir_answer?
    receiveMessageResponse->set_pir_answer(answer_string);

    // return empty Status
    return Status::OK;
  }

 public:
  MessengerImpl(shared_ptr<PirDB> pir_db_ptr) : pir_db(pir_db_ptr) {}

 private:
  shared_ptr<PirDB> pir_db;
};