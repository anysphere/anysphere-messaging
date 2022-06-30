#pragma once

#include "asphr/asphr.hpp"

// A simple database with no security
// TODO: storing public key here is meaningless
// but it is required by libsodium.
// If we want to opimize friend_request space,
// we can try to remove this.
class AsyncInvitationDatabase {
 public:
  AsyncInvitationDatabase()
      : invitations({}), invitation_public_key({}) {};

  void register_user(size_t requester_index, const std::string& public_key);
  void set_invitation(size_t requester_index, const std::string& invitation);

  string get_invitation(size_t requester_index);

  string get_friend_public_key(size_t requester_index);

 private:
  std::map<size_t, string> invitations{};
  // a temporary PKI for the friend request
  // users do not actually need to trust this PKI.
  std::map<size_t, string> invitation_public_key{};
};
