//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#pragma once

#include <sodium.h>

#include <stdexcept>
#include <string>

#include "asphr/schema/asyncinvitation.pb.h"
#include "asphr/schema/message.pb.h"
#include "constants.hpp"

/* Crypto implements an IND-CCA2 secure scheme.

Currently, we do not support forward secrecy. Note that this really only becomes
a problem if we want to be able to delete messages, because otherwise an
attacker that compromises the keys will also be able to read messages.
*/
namespace crypto {

// MUST be called before any other function.
inline auto init() -> void {
  if (sodium_init() < 0) {
    throw std::runtime_error("sodium_init failed");
  }
}

auto generic_hash(string_view data) -> std::string;

// Generates a new keypair in the format <public_key, private_key>.
auto generate_kx_keypair() -> std::pair<string, string>;

auto generate_invitation_keypair() -> std::pair<string, string>;

auto derive_read_write_keys(string my_public_key, string my_private_key,
                            string friend_public_key)
    -> std::pair<string, string>;

// note: it is CRUCIAL that we use a new random nonce EVERY SINGLE TIME.
// for message integrity and indistinguishability this is not a problem,
// but for privacy this is very very important — because if we use a nonce
// that is non-random, an attacker can observe when the nonce follows the
// pattern and when it does not, and then figure out when a user switches
// from talking to one friend to talking to another.
//
// this highlights that we need something stronger than IND-CCA2,
// actually. we also need a ciphertext from one key to be
// indistinguishable from a ciphertext from another key. in other words,
// we need key privacy. Block-cipher based symmetric-key encryption
// schemes do support this out of the box, but it is important to keep in
// mind because it is a somewhat nonstandard requirement.
auto encrypt_send(const asphrclient::Chunk& message_in, const string& write_key)
    -> asphr::StatusOr<pir_value_t>;

auto decrypt_receive(const pir_value_t& ciphertext, const string& read_key)
    -> asphr::StatusOr<asphrclient::Chunk>;

// encrypt_ack encrypts the ack_id to the friend
auto encrypt_ack(uint32_t ack_id, const string& write_key)
    -> asphr::StatusOr<string>;
// decrypt_ack undoes encrypt_ack
auto decrypt_ack(const string& ciphertext, const string& read_key)
    -> asphr::StatusOr<uint32_t>;

// encrypt an asynchronous friend request
// The async friend request from A->B is Enc(ID_B, ID_A || msg) = Enc(f_pk_B,
// ID_A || msg)
auto encrypt_async_invitation(const string& self_id,
                              const string& self_invitation_private_key,
                              const string& friend_invitation_public_key,
                              const string& message) -> asphr::StatusOr<string>;

// this method is needed for transmission retrieval
// WARNING: WHEN CALLING THIS, VERIFY THAT THE RETURNED PUBLIC_ID CORRESPONDS TO
// THE FRIEND_INVITATION_PUBLIC_KEY OTHERWISE, SOMEONE CAN IMPERSONATE A GIVEN
// PUBLIC ID.
auto decrypt_async_invitation(const string& self_invitation_private_key,
                              const string& friend_invitation_public_key,
                              const string& ciphertext)
    -> asphr::StatusOr<pair<string, string>>;
};  // namespace crypto
