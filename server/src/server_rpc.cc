#include "server_rpc.hpp"

#include "beta_key_auth.hpp"
#include "server_constants.hpp"

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
        registerInfo->invitation_public_key(), allocation);
    // todo: register to the friend async request database and "PKI"
    // this might needs to be changed if there are multiple allocations
    async_invitation_database.register_user(
        allocation, registerInfo->invitation_public_key());
    for (auto& alloc : allocation_vec) {
      registerResponse->add_allocation(alloc);
    }
    registerResponse->set_public_key(registerInfo->invitation_public_key());
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

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// |||                             friend_request                           |||
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::AddAsyncInvitation(
    ServerContext* context,
    const AddAsyncInvitationInfo* addAsyncInvitationInfo,
    AddAsyncInvitationResponse* addAsyncInvitationResponse) {
  ASPHR_LOG_INFO("AddAsyncInvitation() called", rpc_call, "AddAsyncInvitation");
  auto index = addAsyncInvitationInfo->index();

  pir_index_t pir_index = index;
  try {
    if (!account_manager.valid_index_access(
            addAsyncInvitationInfo->authentication_token(), pir_index)) {
      ASPHR_LOG_ERR("Incorrect authentication token.", rpc_call,
                    "AddAsyncInvitation", authentication_token,
                    addAsyncInvitationInfo->authentication_token(), index,
                    index);
      return Status(grpc::StatusCode::UNAUTHENTICATED,
                    "incorrect authentication token");
    }
  } catch (const AccountManagerException& e) {
    ASPHR_LOG_ERR("AccountManagerException: ", exception, e.what());
    return Status(grpc::StatusCode::UNAVAILABLE, e.what());
  }

  auto invitation =
      addAsyncInvitationInfo->invitation();  // this is now a byte array

  auto invitation_public_key =
      addAsyncInvitationInfo->invitation_public_key();  // this is the public
                                                        // key of the inviter.

  if (std::ssize(invitation) > MAX_INVITATION_LENGTH) {
    ASPHR_LOG_ERR("Invitation too long.", rpc_call, "AddAsyncInvitation",
                  invitation_length, std::ssize(invitation),
                  max_invitation_length, MAX_INVITATION_LENGTH);
    return Status(grpc::StatusCode::INVALID_ARGUMENT, "invitation too long");
  }

  async_invitation_database.set_invitation(index, std::string(invitation));

  // TODO: move this out and only use the postgress registration instead of
  // doing this everytime.
  async_invitation_database.register_user(index,
                                          std::string(invitation_public_key));

  return Status::OK;
}

template <typename PIR, typename AccountManager>
Status ServerRpc<PIR, AccountManager>::GetAsyncInvitations(
    ServerContext* context,
    const GetAsyncInvitationsInfo* getAsyncInvitationsInfo,
    GetAsyncInvitationsResponse* getAsyncInvitationsResponse) {
  ASPHR_LOG_INFO("Server rpc: GetAsyncInvitations() called");
  // cast to an int to make sure computation doesn't overflow/underflow
  auto start_index = static_cast<int>(getAsyncInvitationsInfo->start_index());
  auto end_index = static_cast<int>(getAsyncInvitationsInfo->end_index());

  if (start_index < 0) {
    ASPHR_LOG_ERR("start_index is negative");
    return Status(grpc::StatusCode::INVALID_ARGUMENT,
                  "start_index is negative");
  }

  if (end_index - start_index > MAX_ASYNC_INVITATION_BATCH_SIZE) {
    ASPHR_LOG_ERR("end_index - start_index too large")
    return Status(grpc::StatusCode::INVALID_ARGUMENT,
                  "Attempt to download too many requests");
  }

  if (start_index > end_index) {
    ASPHR_LOG_ERR("start_index > end_index");
    return Status(grpc::StatusCode::INVALID_ARGUMENT,
                  "start_index > end_index");
  }

  for (auto index = start_index; index < end_index; index++) {
    string request = async_invitation_database.get_invitation(index);
    string invitation_public_key =
        async_invitation_database.get_friend_public_key(index);
    // Do we need to return fake friend requests for unregistered
    // entries? No we do not, cause the server can potentially do anything in
    // our model. We are only exposing which slots are registered by not
    // returning fake request, which for now can be easily achieved
    // by registering a new account.
    getAsyncInvitationsResponse->add_invitations(request);
    getAsyncInvitationsResponse->add_invitation_public_key(
        invitation_public_key);
  }

  return Status::OK;
}