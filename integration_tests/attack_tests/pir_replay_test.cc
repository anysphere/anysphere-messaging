//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "pir_replay_test.hpp"

using namespace asphrdaemon;

/**
 * Things to test:
 * - the attacker template is working.
 **/

namespace asphr::testing {
namespace {

using TestReplayClass = class AttackTest<PIRReplayAttacker>;

// Setup:
// - There are 3 users A, B, C, with index 0, 1, 2.
// - A and C are friends, and A and B might be friends.
// - C is colluding with the server, and wants to know if A and B are friends.
// Rules:
// - server cannot run malicious code before attack_starts() is called.
// - C cannot access the secret bit b.
// - Anything else C and server do is fair game.

TEST_F(TestReplayClass, PIRReplayAttack) {
  ResetStub();
  const int NUM_ITER = 8;
  int NUM_CORRECT = 0;
  // because the random number generator is behaving weirdly, we fix a sequence
  // of coin throws.
  const vector<int> coins = {0, 1, 0, 0, 1, 1, 0, 1};
  // Each iteration of the loop runs a complete setup -> attack loop.
  for (int i = 0; i < NUM_ITER; i++) {
    // ================================================================
    // Setup Phase: A and C are friends, and A and B might be friends.
    // The server is guaranteed to be honest in this phase.
    // ================================================================
    vector<pair<int, int>> friend_list = {{0, 2}};
    // b ~ U([0,1]) is the secret bit
    // determining if A and B are friends or not.
    // const int b = absl::Uniform(absl::IntervalClosed, bitgen_, 0, 1);
    const int b = coins[i];
    if (b == 1) {
      friend_list.push_back({0, 1});
    }
    auto friends = generate_friends_from_list_of_pairs(3, friend_list);
    auto& A = friends.at(0);
    auto& B = friends.at(1);
    auto& C = friends.at(2);
    // tells the server the index of A, B, C.
    auto A_send_info = A.G->db->get_send_info();
    auto B_send_info = B.G->db->get_send_info();
    auto C_send_info = C.G->db->get_send_info();
    int A_index = A_send_info.allocation;
    int B_index = B_send_info.allocation;
    int C_index = C_send_info.allocation;
    ASPHR_LOG_INFO("Indices: ", A_index, A_index, B_index, B_index, C_index,
                   C_index);
    attacker_service_.attack_starts(A_index, B_index, C_index);
    // ================================================================
    // We assume the server is honest up to this point.
    // After this point, the server and C can do whatever they want.
    // ================================================================
    {
      // C formulates a test message
      // (After about 30 minutes I have given up on manually forming a valid
      // message).
      // (I'll just go through C's daemon and let the server retrieve the
      // message during the SendMessage() RPC.)
      asphrdaemon::SendMessageRequest request;
      request.add_unique_name(A.unique_name);
      request.set_message("Hello~~Right now~~I am behind you~~");
      asphrdaemon::SendMessageResponse response;
      auto status = C.rpc->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
      C.t->send();
    }

    if (b == 1) {
      // A sends B a message if they are friends
      // This is to make the result more deterministic,
      // as we now know for certain A will retrieve from B immediately after
      // this.
      asphrdaemon::SendMessageRequest request;
      request.add_unique_name(B.unique_name);
      request.set_message("hello from A to B");
      asphrdaemon::SendMessageResponse response;
      auto status = A.rpc->SendMessage(nullptr, &request, &response);
      EXPECT_TRUE(status.ok());
    }

    // To save time, we run A the minimal amount of time to get the result.
    // during the first run, A will leak a valid PIR query
    // which could be exploited to run a replay attack
    // during the first retrieve call.
    A.t->send();
    A.t->retrieve();
    // A will now send an ACK to C.
    A.t->send();
    // C should now be able to see the ACK!
    C.t->retrieve();
    // if everything goes well, we can now guess b' by reading the ack index for
    // C. We have to venture into the db for that.
    int b_guess;
    {
      // C produce a guess of b
      // we can now guess b by reading the ack_seq_num for C!!
      auto friend_struct = C.G->db->get_friend(A.unique_name);
      // A is C's only friend.
      assert(friend_struct.uid == 1);
      auto ack_seq_num = C.G->db->test_only_get_ack_seq_num(friend_struct.uid);
      b_guess = ack_seq_num;
    }
    // check C's guess
    // end attack
    ASPHR_LOG_INFO("Attack result: ", b, b, b_guess, b_guess);
    if (b_guess == b) {
      NUM_CORRECT++;
      ASPHR_LOG_INFO("Correct guess!");
    } else {
      ASPHR_LOG_INFO("Wrong guess!");
    }
    attacker_service_.attack_ends();
    ASPHR_LOG_INFO(
        "===========================================================");
    ASPHR_LOG_INFO(
        "=====================End of Attack=========================");
    ASPHR_LOG_INFO(
        "===========================================================");
  }
  // log the result.
  ASPHR_LOG_INFO("===========================================================");
  ASPHR_LOG_INFO("======================End of Test==========================");
  ASPHR_LOG_INFO("===========================================================");
  ASPHR_LOG_INFO("TEST RESULT: ", num_attack, NUM_ITER, num_correct,
                 NUM_CORRECT);
  // if attacker gets the correct guess with overwhelming probability, something
  // is wrong
  if (NUM_CORRECT >= NUM_ITER - 1) {
    FAIL();
  }
}
}  // namespace
}  // namespace asphr::testing