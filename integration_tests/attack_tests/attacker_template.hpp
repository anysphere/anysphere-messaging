//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "../daemon_setup.hpp"

namespace asphr::testing {

namespace {

// A template for testing the attacker.
template <class AttackerClass>
class AttackTest : public DaemonRpcTest {
  // override reset_stub() to reset the stub to the attack server.
  // using Attacker = class AttackerClass<FastPIR, AccountManagerInMemory>;
 public:
  AttackTest() : attacker_service_(AttackerClass()) {}

  void ResetStub() {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(
        server_address_.str(), grpc::InsecureChannelCredentials());
    stub_ = asphrserver::Server::NewStub(channel);
  }

  void SetUp() {
    // get a random port number
    // https://github.com/yegor256/random-tcp-port might be useful sometime in
    // the future.
    const int port = absl::Uniform(bitgen_, 10'000, 65'000);
    server_address_ << "localhost:" << port;
    // Setup server
    grpc::ServerBuilder builder;
    ASPHR_LOG_INFO("Server initializing",
                   "server address: ", server_address_.str());
    builder.AddListeningPort(server_address_.str(),
                             grpc::InsecureServerCredentials());
    builder.RegisterService(&attacker_service_);
    server_ = builder.BuildAndStart();
  }

  void TearDown() {
    attacker_service_.shutdown_honest_server();
    DaemonRpcTest::TearDown();
  }

  // need to change the service_ variable a bit
  // to make it work with the attack server.

  // The old service_ variable is no longer used.
  // If we use it anywhere in daemon_setup
  // we need to change it as well.
  AttackerClass attacker_service_;
};

// A basic attacker that only outputs "Hello from MITM!"
// And answer query honestly.
// All attackers overrides this template.
template <typename PIR, typename AccountManager>
class AttackerTemplate : public asphrserver::Server::Service {
  using pir_query_t = typename PIR::pir_query_t;
  using pir_answer_t = typename PIR::pir_answer_t;
  using ServerRpc = ServerRpc<PIR, AccountManager>;
  // the ServerRpc Class is final for good reasons.
  // So what the attack server does is that it spins up a new ServerRpc instance
  // and acts as a MITM between the server and the client. The method signatures
  // are the same as the ones in the ServerRpc class.
 public:
  grpc::Status Register(grpc::ServerContext* context,
                        const asphrserver::RegisterInfo* registerInfo,
                        asphrserver::RegisterResponse* registerResponse) {
    ASPHR_LOG_INFO("Hello from MITM!");
    // forward everything to honest server
    grpc::ClientContext client_context;
    auto status = honest_stub_->Register(&client_context, *registerInfo,
                                         registerResponse);
    return status;
  }

  grpc::Status SendMessage(
      grpc::ServerContext* context,
      const asphrserver::SendMessageInfo* sendMessageInfo,
      asphrserver::SendMessageResponse* sendMessageResponse) {
    // forward everything to honest server
    ASPHR_LOG_INFO("Hello from MITM!");
    grpc::ClientContext client_context;
    auto status = honest_stub_->SendMessage(&client_context, *sendMessageInfo,
                                            sendMessageResponse);
    return status;
  }

  grpc::Status ReceiveMessage(
      grpc::ServerContext* context,
      const asphrserver::ReceiveMessageInfo* receiveMessageInfo,
      asphrserver::ReceiveMessageResponse* receiveMessageResponse) {
    // forward everything to honest server
    ASPHR_LOG_INFO("Hello from MITM!");
    grpc::ClientContext client_context;
    auto status = honest_stub_->ReceiveMessage(
        &client_context, *receiveMessageInfo, receiveMessageResponse);
    return status;
  }

  grpc::Status AddAsyncInvitation(
      grpc::ServerContext* context,
      const asphrserver::AddAsyncInvitationInfo* addAsyncInvitationInfo,
      asphrserver::AddAsyncInvitationResponse* addAsyncInvitationResponse) {
    // forward everything to honest server
    ASPHR_LOG_INFO("Hello from MITM!");
    grpc::ClientContext client_context;
    auto status = honest_stub_->AddAsyncInvitation(
        &client_context, *addAsyncInvitationInfo, addAsyncInvitationResponse);
    return status;
  }

  grpc::Status GetAsyncInvitations(
      grpc::ServerContext* context,
      const asphrserver::GetAsyncInvitationsInfo* getAsyncInvitationsInfo,
      asphrserver::GetAsyncInvitationsResponse* getAsyncInvitationsResponse) {
    // forward everything to honest server
    ASPHR_LOG_INFO("Hello from MITM!");
    grpc::ClientContext client_context;
    auto status = honest_stub_->GetAsyncInvitations(
        &client_context, *getAsyncInvitationsInfo, getAsyncInvitationsResponse);
    return status;
  }

  // Initializer: initializes the honest server as well.
  AttackerTemplate() : honest_service_(gen_server_rpc()) {
    // initializes a ServerRpc stub for the honest server.
    const int honest_port = absl::Uniform(bitgen_, 10'000, 65'000);
    honest_server_address_ << "localhost:" << honest_port;
    // Setup server
    grpc::ServerBuilder builder;
    ASPHR_LOG_INFO("Server initializing",
                   "server address: ", honest_server_address_.str());
    builder.AddListeningPort(honest_server_address_.str(),
                             grpc::InsecureServerCredentials());
    builder.RegisterService(&honest_service_);
    honest_server_ = builder.BuildAndStart();
    honest_stub_ = asphrserver::Server::NewStub(grpc::CreateChannel(
        honest_server_address_.str(), grpc::InsecureChannelCredentials()));
  }

  auto shutdown_honest_server() { honest_server_->Shutdown(); }

 private:
  // variables controlling the honest server
  absl::BitGen bitgen_;
  std::shared_ptr<asphrserver::Server::Stub> honest_stub_;
  std::unique_ptr<grpc::Server> honest_server_;
  std::ostringstream honest_server_address_;
  ServerRpc honest_service_;
};
}  // namespace
}  // namespace asphr::testing
