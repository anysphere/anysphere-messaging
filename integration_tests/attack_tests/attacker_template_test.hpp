#include "attacker_template.hpp"

// A simple attacker that just DoS all messages
namespace asphr::testing {
namespace {
template <typename PIR, typename AccountManager>
class DoSAttacker : public AttackerTemplate<PIR, AccountManager> {
 public:
  DoSAttacker() : AttackerTemplate<PIR, AccountManager>() {}

  // override SendMessage() to do nothing
  grpc::Status SendMessage(
      grpc::ServerContext* context,
      const asphrserver::SendMessageInfo* sendMessageInfo,
      asphrserver::SendMessageResponse* sendMessageResponse) {
    return grpc::Status::OK;
  }
};
}  // namespace
}  // namespace asphr::testing