//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//


#include "async_invitation_database.hpp"

void AsyncInvitationDatabase::register_user(
    size_t requester_index, const std::string& public_key) {
  if (invitation_public_key.find(requester_index) !=
      invitation_public_key.end()) {
    ASPHR_LOG_ERR("Repeated Registration!", requester_index, requester_index);
  }
  // TODO: move this to a PKI
  invitation_public_key.insert_or_assign(requester_index, public_key);
}
void AsyncInvitationDatabase::set_invitation(
    size_t requester_index, const std::string& invitation) {
  if (invitation_public_key.find(requester_index) ==
      invitation_public_key.end()) {
    ASPHR_LOG_ERR("The index is not registered...", requester_index,
                  requester_index);
  }
  invitations.insert_or_assign(requester_index, invitation);
}

string AsyncInvitationDatabase::get_invitation(size_t requester_index) {
  // if requester_id does not exist, return
  if (invitations.find(requester_index) == invitations.end()) {
    // it's fine for the server to return something with the wrong format here.
    // yes, users can figure out how many users are in the database, but that's
    // it. they could already figure that out. (by for example creating a new
    // account and looking at the allocation) so this is all fine. in fact, the
    // user SHOULD know how many users are registered, because the number of
    // users directly correlates with the level of privacy that is achieved.
    // more users => better privacy.
    return "Not found";
  }
  // return the request at requester_id
  return invitations.at(requester_index);
}

string AsyncInvitationDatabase::get_friend_public_key(size_t requester_index) {
  // if requester_id does not exist, return
  if (invitation_public_key.find(requester_index) ==
      invitation_public_key.end()) {
    return "Not found";
  }
  // return the request at requester_id
  return invitation_public_key.at(requester_index);
}
