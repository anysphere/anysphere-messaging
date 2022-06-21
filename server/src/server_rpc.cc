#include "server_rpc.hpp"

#include "beta_key_auth.hpp"

using grpc::ServerContext;
using grpc::Status;

using namespace asphrserver;

template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::Register(
    ServerContext* context, const RegisterInfo* registerInfo,
    RegisterResponse* registerResponse) {
  ASPHR_LOG_INFO("Register() called.", rpc_call, "Register");
  try {
    auto beta_key = registerInfo->beta_key();

    if (!beta_key_authenticator(beta_key)) {
      ASPHR_LOG_ERR("Beta key is not valid.", rpc_call, "Register", beta_key,
                    beta_key);
      return Status(grpc::StatusCode::UNAUTHENTICATED, "beta_key is invalid");
    }

    // TODO: allocate in a loop
    auto allocation = pir.allocate();
    auto acks_allocation = pir_acks.allocate();
    (void)acks_allocation;
    assert(allocation == acks_allocation);
    auto [auth_token, allocation_vec] = account_manager.generate_account(
        registerInfo->public_key(), allocation);
    for (auto& alloc : allocation_vec) {
      registerResponse->add_allocation(alloc);
    }
    registerResponse->set_public_key(registerInfo->public_key());
    registerResponse->set_authentication_token(auth_token);
  } catch (const AccountManagerException& e) {
    ASPHR_LOG_ERR("Could not access account maanger.", error, e.what(),
                  rpc_call, "Register");
    return Status(grpc::StatusCode::UNAVAILABLE, e.what());
  }

  return Status::OK;
}

template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::SendMessage(
    ServerContext* context, const SendMessageInfo* sendMessageInfo,
    SendMessageResponse* sendMessageResponse) {
  auto index = sendMessageInfo->index();
  ASPHR_LOG_INFO("SendMessage() called.", rpc_call, "SendMessage",
                 index_to_write_message_to, index);
  pir_index_t pir_index = index;
  try {
    if (!account_manager.valid_index_access(
            sendMessageInfo->authentication_token(), pir_index)) {
      ASPHR_LOG_ERR("Incorrect authentication token.", rpc_call, "SendMessage",
                    index_to_write_message_to, index);
      return Status(grpc::StatusCode::UNAUTHENTICATED,
                    "incorrect authentication token");
    }
  } catch (const AccountManagerException& e) {
    ASPHR_LOG_ERR("Could not access account maanger.", error, e.what(),
                  rpc_call, "SendMessage", index_to_write_message_to, index);
    return Status(grpc::StatusCode::UNAVAILABLE, e.what());
  }

  auto message = sendMessageInfo->message();
  // size MUST be exactly the same for everyone always for privacy!!!
  if (message.size() != sizeof(pir_value_t)) {
    ASPHR_LOG_ERR("Message size is not the same as the PIR value size.",
                  rpc_call, "SendMessage", index_to_write_message_to, index,
                  message_size, message.size(), pir_value_size,
                  sizeof(pir_value_t));
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "incorrect message size");
  }
  pir_value_t pir_value;
  std::copy(message.begin(), message.end(), pir_value.begin());

  pir.set_value(pir_index, pir_value);

  auto acks = sendMessageInfo->acks();
  if (acks.size() != sizeof(pir_value_t)) {
    ASPHR_LOG_ERR("Acks size is not the same as the PIR value size.", rpc_call,
                  "SendMessage", index_to_write_message_to, index, acks_size,
                  acks.size(), pir_value_size, sizeof(pir_value_t));
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "incorrect acks size");
  }
  pir_value_t pir_value_acks;
  std::copy(acks.begin(), acks.end(), pir_value_acks.begin());
  pir_acks.set_value(pir_index, pir_value_acks);

  return Status::OK;
}

template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::ReceiveMessage(
    ServerContext* context, const ReceiveMessageInfo* receiveMessageInfo,
    ReceiveMessageResponse* receiveMessageResponse) {
  ASPHR_LOG_INFO("ReceiveMessage() called.", rpc_call, "ReceiveMessage");
  auto input_query = receiveMessageInfo->pir_query();
  // TODO: check that input_query is not too long

  pir_query_t query;
  try {
    query = pir.query_from_string(input_query);
  } catch (const std::runtime_error& e) {
    ASPHR_LOG_ERR("Could not parse query.", error, e.what(), rpc_call,
                  "ReceiveMessage", query_to_receive, input_query);
    return Status(grpc::StatusCode::INTERNAL, e.what());
  } catch (const std::invalid_argument& e) {
    ASPHR_LOG_ERR("Could not parse query.", error, e.what(), rpc_call,
                  "ReceiveMessage", query_to_receive, input_query);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  } catch (const std::logic_error& e) {
    ASPHR_LOG_ERR("Could not parse query.", error, e.what(), rpc_call,
                  "ReceiveMessage", query_to_receive, input_query);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  pir_answer_t answer;
  try {
    answer = pir.get_value_privately(query);
  } catch (const std::invalid_argument& e) {
    ASPHR_LOG_ERR("Could not get PIR answer.", error, e.what(), rpc_call,
                  "ReceiveMessage");
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  string answer_string;
  try {
    answer_string = answer.serialize_to_string();
  } catch (const std::runtime_error& e) {
    ASPHR_LOG_ERR("Could not serialize answer.", error, e.what(), rpc_call,
                  "ReceiveMessage", answer_to_receive, answer_string);
    return Status(grpc::StatusCode::INTERNAL, e.what());
  } catch (const std::invalid_argument& e) {
    ASPHR_LOG_ERR("Could not serialize answer.", error, e.what(), rpc_call,
                  "ReceiveMessage", answer_to_receive, answer_string);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  } catch (const std::logic_error& e) {
    ASPHR_LOG_ERR("Could not serialize answer.", error, e.what(), rpc_call,
                  "ReceiveMessage", answer_to_receive, answer_string);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  receiveMessageResponse->set_pir_answer(std::move(answer_string));

  pir_answer_t answer_acks;
  try {
    answer_acks = pir_acks.get_value_privately(query);
  } catch (const std::invalid_argument& e) {
    ASPHR_LOG_ERR("Could not get PIR answer for the acks.", error, e.what(),
                  rpc_call, "ReceiveMessage");
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  string answer_acks_string;
  try {
    answer_acks_string = answer_acks.serialize_to_string();
  } catch (const std::runtime_error& e) {
    ASPHR_LOG_ERR("Could not serialize answer for the acks.", error, e.what(),
                  rpc_call, "ReceiveMessage", answer_acks_to_receive,
                  answer_acks_string);
    return Status(grpc::StatusCode::INTERNAL, e.what());
  } catch (const std::invalid_argument& e) {
    ASPHR_LOG_ERR("Could not serialize answer for the acks.", error, e.what(),
                  rpc_call, "ReceiveMessage", answer_acks_to_receive,
                  answer_acks_string);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  } catch (const std::logic_error& e) {
    ASPHR_LOG_ERR("Could not serialize answer for the acks.", error, e.what(),
                  rpc_call, "ReceiveMessage", answer_acks_to_receive,
                  answer_acks_string);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, e.what());
  }

  receiveMessageResponse->set_pir_answer_acks(std::move(answer_acks_string));

  return Status::OK;
}