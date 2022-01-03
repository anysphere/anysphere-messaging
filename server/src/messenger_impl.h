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

#include "pir/pir_common.h"
#include "account_manager.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using messenger::Messenger;

using std::cout;
using std::endl;
using std::string;

template <typename PIR, typename AccountManager>
class MessengerImpl final : public Messenger::Service
{
  using pir_query_t = typename PIR::pir_query_t;
  using pir_answer_t = typename PIR::pir_answer_t;
  // TODO: add a thread safety argument (because the methods may be called from
  // different threads)
  // TODO: add representation invariant
  Status Register(ServerContext *context,
                  const messenger::RegisterInfo *registerInfo,
                  messenger::RegisterResponse *registerResponse) override
  {
    cout << "Register() called" << endl;
    try
    {
      // TODO: allocate in a loop
      auto allocation = pir.allocate();
      account_manager.generate_account(registerInfo->public_key(), allocation);
    }
    catch (const AccountManagerException &e)
    {
      std::cerr << "AccountManagerException: " << e.what() << std::endl;
      return Status(grpc::StatusCode::UNAVAILABLE, e.what());
    }

    return Status::OK;
  }

  Status SendMessage(
      ServerContext *context, const messenger::SendMessageInfo *sendMessageInfo,
      messenger::SendMessageResponse *sendMessageResponse) override
  {
    // TODO: make this into actual logs using actual structured logging
    cout << "SendMessage() called" << endl;
    auto index = sendMessageInfo->index();
    cout << "index: " << index << endl;
    pir_index_t pir_index = index;
    try
    {
      if (account_manager.valid_index_access(sendMessageInfo->authentication_token(), pir_index))
      {
        std::cerr << "incorrect authentication token" << std::endl;
        return Status(grpc::StatusCode::UNAUTHENTICATED, "incorrect authentication token");
      }
    }
    catch (const AccountManagerException &e)
    {
      std::cerr << "AccountManagerException: " << e.what() << std::endl;
      return Status(grpc::StatusCode::UNAVAILABLE, e.what());
    }

    cout << "send message index: " << index << endl;

    auto message = sendMessageInfo->message();
    if (message.size() != sizeof(pir_value_t))
    {
      std::cerr << "incorrect message size" << std::endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "incorrect message size");
    }
    pir_value_t pir_value;
    std::copy(message.begin(), message.end(), pir_value.begin());

    pir.set_value(pir_index, pir_value);

    return Status::OK;
  }

  Status ReceiveMessage(
      ServerContext *context,
      const messenger::ReceiveMessageInfo *receiveMessageInfo,
      messenger::ReceiveMessageResponse *receiveMessageResponse) override
  {
    cout << "ReceiveMessage() called" << endl;
    auto input_query = receiveMessageInfo->pir_query();
    // TODO: check that input_query is not too long

    pir_query_t query;
    try
    {
      query = pir.query_from_string(input_query);
    }
    catch (const std::runtime_error &e)
    {
      std::cerr << "runtime_error: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INTERNAL, e.what());
    }
    catch (const std::invalid_argument &e)
    {
      std::cerr << "invalid_argument: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
    catch (const std::logic_error &e)
    {
      std::cerr << "logic_error: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }

    pir_answer_t answer;
    try
    {
      answer = pir.get_value_privately(query);
    }
    catch (const std::invalid_argument &e)
    {
      std::cerr << "invalid_argument: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }

    string answer_string;
    try
    {
      answer_string = answer.serialize_to_string();
    }
    catch (const std::runtime_error &e)
    {
      std::cerr << "runtime_error: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INTERNAL, e.what());
    }
    catch (const std::invalid_argument &e)
    {
      std::cerr << "invalid_argument: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
    catch (const std::logic_error &e)
    {
      std::cerr << "logic_error: " << e.what() << std::endl;
      return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }

    receiveMessageResponse->set_pir_answer(std::move(answer_string));

    return Status::OK;
  }

public:
  MessengerImpl(PIR &pir, AccountManager &account_manager) : pir(pir), account_manager(account_manager) {}

private:
  PIR &pir;
  AccountManager &account_manager;
};