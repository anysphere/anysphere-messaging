#include "server_rpc.hpp"

using grpc::ServerContext;
using grpc::Status;

using namespace asphrserver;

template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::Register(
    ServerContext* context, const RegisterInfo* registerInfo,
    RegisterResponse* registerResponse) {
  cout << "Register() called" << endl;
  try {
    // TODO: allocate in a loop
    auto allocation = pir.allocate();
    auto [auth_token, allocation_vec] = account_manager.generate_account(
        registerInfo->public_key(), allocation);

    for (auto& alloc : allocation_vec) {
      registerResponse->add_allocation(alloc);
    }
    registerResponse->set_public_key(registerInfo->public_key());
    registerResponse->set_authentication_token(auth_token);
  } catch (const AccountManagerException& e) {
    std::cerr << "AccountManagerException: " << e.what() << std::endl;
    return Status(grpc::StatusCode::UNAVAILABLE, e.what());
  }

  return Status::OK;
}

template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::SendMessage(
    ServerContext* context, const SendMessageInfo* sendMessageInfo,
    SendMessageResponse* sendMessageResponse) {
  // TODO: make this into actual logs using actual structured logging
  cout << "SendMessage() called" << endl;
  auto index = sendMessageInfo->index();
  cout << "index: " << index << endl;
  pir_index_t pir_index = index;
  try {
    if (!account_manager.valid_index_access(
            sendMessageInfo->authentication_token(), pir_index)) {
      cerr << "incorrect authentication token" << endl;
      return Status(grpc::StatusCode::UNAUTHENTICATED,
                    "incorrect authentication token");
    }
  } catch (const AccountManagerException& e) {
    cerr << "AccountManagerException: " << e.what() << endl;
    return Status(grpc::StatusCode::UNAVAILABLE, e.what());
  }

  cout << "send message index: " << index << endl;

  auto message = sendMessageInfo->message();
  // size MUST be exactly the same for everyone always for privacy!!!
  if (message.size() != sizeof(pir_value_t)) {
    cerr << "incorrect message size" << endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "incorrect message size");
  }
  pir_value_t pir_value;
  std::copy(message.begin(), message.end(), pir_value.begin());

  pir.set_value(pir_index, pir_value);

  auto db_rows = pir.get_db_rows();
  sendMessageResponse->set_db_rows(db_rows);

  return Status::OK;
}

template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::ReceiveMessage(
    ServerContext* context, const ReceiveMessageInfo* receiveMessageInfo,
    ReceiveMessageResponse* receiveMessageResponse) {
  cout << "ReceiveMessage() called" << endl;
  auto input_query = receiveMessageInfo->pir_query();
  // TODO: check that input_query is not too long

  pir_query_t query;
  try {
    query = pir.query_from_string(input_query);
  } catch (const std::runtime_error& e) {
    std::cerr << "runtime_error: " << e.what() << std::endl;
    return Status(grpc::StatusCode::INTERNAL, e.what());
  } catch (const std::invalid_argument& e) {
    std::cerr << "invalid_argument: " << e.what() << std::endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  } catch (const std::logic_error& e) {
    std::cerr << "logic_error: " << e.what() << std::endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  pir_answer_t answer;
  try {
    answer = pir.get_value_privately(query);
  } catch (const std::invalid_argument& e) {
    std::cerr << "invalid_argument: " << e.what() << std::endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  string answer_string;
  try {
    answer_string = answer.serialize_to_string();
  } catch (const std::runtime_error& e) {
    std::cerr << "runtime_error: " << e.what() << std::endl;
    return Status(grpc::StatusCode::INTERNAL, e.what());
  } catch (const std::invalid_argument& e) {
    std::cerr << "invalid_argument: " << e.what() << std::endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  } catch (const std::logic_error& e) {
    std::cerr << "logic_error: " << e.what() << std::endl;
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  receiveMessageResponse->set_pir_answer(std::move(answer_string));

  return Status::OK;
}