#include <grpcpp/grpcpp.h>

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#ifdef BAZEL_BUILD
#include "schema/messenger.grpc.pb.h"
#else
#include "schema/messenger.grpc.pb.h"
#endif

#include "pir_common.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using messenger::Messenger;

using std::string;

template<typename PIR>
class MessengerImpl final : public Messenger::Service {
  using pir_query_t = typename PIR::pir_query_t;
  using pir_answer_t = typename PIR::pir_answer_t;
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
    auto index = sendMessageInfo->index();
    pir_index_t pir_index = index;
    if (sendMessageInfo->authentication_token() != "hello") {
      std::cerr << "incorrect authentication token" << std::endl;
      return Status::CANCELLED;
    }

    auto message = sendMessageInfo->message();
    if (message.size() != sizeof(pir_value_t)) {
      std::cerr << "incorrect message size" << std::endl;
      return Status::CANCELLED;
    }
    pir_value_t pir_value;
    std::copy(message.begin(), message.end(), pir_value.begin());

    pir.set_value(pir_index, pir_value);

    return Status::OK;
  }

  Status ReceiveMessage(
      ServerContext *context,
      const messenger::ReceiveMessageInfo *receiveMessageInfo,
      messenger::ReceiveMessageResponse *receiveMessageResponse) override {

    auto input_query = receiveMessageInfo->pir_query();
    pir_query_t query;
    bool success = query.deserialize_from_string(input_query);
    if (!success) {
      std::cerr << "error deserializing query" << std::endl;
      return Status::CANCELLED;
    }

    pir_answer_t answer = pir.get_value_privately(query);

    string answer_string = answer.serialize_to_string();

    receiveMessageResponse->set_pir_answer(std::move(answer_string));

    return Status::OK;
  }

 public:
  MessengerImpl(PIR & pir) : pir(pir) {}

 private:
  PIR & pir;
};