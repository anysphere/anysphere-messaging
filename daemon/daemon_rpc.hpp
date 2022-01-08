
#include "config.hpp"
#include "schema/daemon.grpc.pb.h"

class DaemonImpl final : public Daemon::Service {
 public:
  Status RegisterUser(
      ServerContext* context,
      const Daemon::RegisterUserRequest* registerUserRequest,
      Daemon::RegisterUserResponse* registerUserResponse) override;

  Status GetFriendList(
      ServerContext* context,
      const Daemon::GetFriendListRequest* getFriendListRequest,
      Daemon::GetFriendListResponse* getFriendListResponse) override;

  Status GenerateFriendKey(
      ServerContext* context,
      const Daemon::GenerateFriendKeyRequest* generateFriendKeyRequest,
      Daemon::GenerateFriendKeyResponse* generateFriendKeyResponse) override;

  Status AddFriend(ServerContext* context,
                   const Daemon::AddFriendRequest* addFriendRequest,
                   Daemon::AddFriendResponse* addFriendResponse) override;

  Status RemoveFriend(
      ServerContext* context,
      const Daemon::RemoveFriendRequest* removeFriendRequest,
      Daemon::RemoveFriendResponse* removeFriendResponse) override;

  Status SendMessage(ServerContext* context,
                     const Daemon::SendMessageRequest* sendMessageRequest,
                     Daemon::SendMessageResponse* sendMessageResponse) override;

  Status GetAllMessages(
      ServerContext* context,
      const Daemon::GetAllMessagesRequest* getAllMessagesRequest,
      Daemon::GetAllMessagesResponse* getAllMessagesResponse) override;

 private:
  Crypto& crypto;
  Config& config;
};