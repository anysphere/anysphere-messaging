#include <gtest/gtest.h>
using testing::Types;

#include "server/src/account_manager.hpp"

using std::string;

template <typename T>
struct AccountManagerTest : public testing::Test {};

typedef Types<AccountManagerInMemory, AccountManagerPostgres> Implementations;

TYPED_TEST_SUITE(AccountManagerTest, Implementations);

// this should pass iff postgres is running! (hence disabled)
TYPED_TEST(AccountManagerTest, DISABLED_Basic) {
  string db_address = "127.0.0.1";
  string db_password = "password";
  TypeParam account_manager(db_address, db_password);

  auto index = 1;
  auto public_key = "public_key";
  auto [auth_token, allocation] =
      account_manager.generate_account(public_key, index);

  // TODO: update this when we allow bigger allocations
  EXPECT_EQ(allocation.size(), 1);

  EXPECT_EQ(allocation[0], index);

  EXPECT_TRUE(account_manager.valid_index_access(auth_token, index));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index + 1));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index - 1));

  EXPECT_FALSE(account_manager.valid_index_access("", index));

  EXPECT_FALSE(account_manager.valid_index_access("public_key1", index));
  EXPECT_FALSE(account_manager.valid_index_access("public_ke", index));
}

TYPED_TEST(AccountManagerTest, DISABLED_RealPublicKey) {
  string db_address = "127.0.0.1";
  string db_password = "password";
  TypeParam account_manager(db_address, db_password);

  auto index = 1;
  char public_key_buffer[2];
  // a real public key has non-utf8 characters
  public_key_buffer[0] = static_cast<char>(0);    // invalid character
  public_key_buffer[0] = static_cast<char>(255);  // possibly invalid character
  string public_key;
  public_key.assign(public_key_buffer, sizeof(public_key_buffer));
  auto [auth_token, allocation] =
      account_manager.generate_account(public_key, index);

  // TODO: update this when we allow bigger allocations
  EXPECT_EQ(allocation.size(), 1);

  EXPECT_EQ(allocation[0], index);

  EXPECT_TRUE(account_manager.valid_index_access(auth_token, index));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index + 1));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index - 1));

  EXPECT_FALSE(account_manager.valid_index_access("", index));

  EXPECT_FALSE(account_manager.valid_index_access("public_key1", index));
  EXPECT_FALSE(account_manager.valid_index_access("public_ke", index));
}