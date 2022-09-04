//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "attacker_template.hpp"

// Simulates a PIR replay attack.
// A lot of stuff are hardcoded here.
namespace asphr::testing {
// the implementation here is specific to the encoding of FastPIR.
namespace {
class PIRReplayAttacker
    : public AttackerTemplate<FastPIR, AccountManagerInMemory> {
  using pir_query_t = FastPIR::pir_query_t;
  using pir_answer_t = FastPIR::pir_answer_t;

 public:
  PIRReplayAttacker() : AttackerTemplate<FastPIR, AccountManagerInMemory>() {
    // initialize local PIR database
    // This allocates 4000 indices.
    // This db is only used to reuse existing pir methods.
    pir.allocate_to_max(4000);
  }

  grpc::Status SendMessage(
      grpc::ServerContext* context,
      const asphrserver::SendMessageInfo* sendMessageInfo,
      asphrserver::SendMessageResponse* sendMessageResponse) {
    // before the attack starts, do everything honestly
    if (!attacking) {
      return AttackerTemplate<FastPIR, AccountManagerInMemory>::SendMessage(
          context, sendMessageInfo, sendMessageResponse);
    }
    ASPHR_LOG_INFO("XxXxXx");
    // ================================================================
    // everything below is fair game!
    // ================================================================
    // Intercept the message from C -> A
    int index = sendMessageInfo->index();
    auto message = sendMessageInfo->message();
    if (send_counter == 0) {
      // this is the test message from C
      EXPECT_EQ(index, C_index_);
      // we insert it into B's spot in the db instead
      if (message.size() != sizeof(pir_value_t)) {
        ASPHR_LOG_ERR("test message size is not correct", message_size,
                      message.size(), expected_message_size,
                      sizeof(pir_value_t));
        throw std::runtime_error("test message size is not correct");
      }
      pir_value_t pir_value;

      std::copy(message.begin(), message.end(), pir_value.begin());

      // dump the pir_value_t in hex
      std::stringstream ss;
      for (auto& c : pir_value) {
        ss << std::hex << (int)c;
      }
      ASPHR_LOG_INFO("test message: " + ss.str());

      pir.set_value(B_index_, pir_value);
      send_counter++;
      return grpc::Status::OK;
    } else {
      // be honest otherwise
      send_counter++;
      return AttackerTemplate<FastPIR, AccountManagerInMemory>::SendMessage(
          context, sendMessageInfo, sendMessageResponse);
    }
  }

  grpc::Status ReceiveMessage(
      grpc::ServerContext* context,
      const asphrserver::ReceiveMessageInfo* receiveMessageInfo,
      asphrserver::ReceiveMessageResponse* receiveMessageResponse) {
    // before the attack starts, do everything honestly
    if (!attacking) {
      return AttackerTemplate<FastPIR, AccountManagerInMemory>::ReceiveMessage(
          context, receiveMessageInfo, receiveMessageResponse);
    }
    // ================================================================
    // everything below is fair game!
    // ================================================================
    // First forward the message to the honest server
    auto status =
        AttackerTemplate<FastPIR, AccountManagerInMemory>::ReceiveMessage(
            context, receiveMessageInfo, receiveMessageResponse);
    if (!status.ok()) {
      throw std::runtime_error("ReceiveMessage failed!!");
    }
    // Now manipulate!
    // We assume that the first four calls to this method will be from A
    if (retrieve_counter == 0) {
      // possibly A -> B
      pir_query_t sus_query;
      try {
        sus_query = pir.query_from_string(receiveMessageInfo->pir_query());
        answer = pir.get_value_privately(sus_query);
        // due to peculiarity of Addra's encoding,
        // the plaintext polynomial is off by one rotation
        // we need to do a rotation to get the polynomial into the desired form.
        assert(B_index_ < 1000 && C_index_ < 1000);
        auto sc = create_context_params();
        auto evaluator = seal::Evaluator(sc);
        evaluator.rotate_rows_inplace(answer.answer, B_index_ - C_index_,
                                      sus_query.galois_keys);
      } catch (std::exception& e) {
        throw std::runtime_error("query / encryption is not correct");
      }
    } else if (retrieve_counter == 1) {
      // A -> C if A and B are friends
      // time for the echo.
      receiveMessageResponse->set_pir_answer(answer.serialize_to_string());
    }
    retrieve_counter++;
    return status;
  }

  void attack_starts(int A_index, int B_index, int C_index) {
    attacking = true;
    send_counter = 0;
    retrieve_counter = 0;
    A_index_ = A_index;
    B_index_ = B_index;
    C_index_ = C_index;
  }

  void attack_ends() { attacking = false; }

  bool attacking = false;
  FastPIR pir;
  int A_index_ = 0;
  int B_index_ = 1;
  int C_index_ = 2;
  int send_counter = 0;
  int retrieve_counter = 0;
  pir_query_t sus_query;  // the query suspected to be A -> B
  pir_answer_t answer;    // the answer to be echoed back next time
};
}  // namespace
}  // namespace asphr::testing