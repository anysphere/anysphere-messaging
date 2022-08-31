#include <gtest/gtest.h>
using testing::Types;

#include "server/src/account_manager.hpp"

using std::string;

template <typename T>
struct AccountManagerTest : public testing::Test {};

#ifdef ENABLE_POSTGRES_TESTS
using Implementations = Types<AccountManagerInMemory, AccountManagerPostgres>;
#else
using Implementations = Types<AccountManagerInMemory>;
#endif

TYPED_TEST_SUITE(AccountManagerTest, Implementations);

// this should pass iff postgres is running! (hence disabled)
TYPED_TEST(AccountManagerTest, Basic) {
  string db_address = "127.0.0.1";
  string db_password = "password";
  TypeParam account_manager(db_address, db_password);

  auto index = 1;
  const auto* public_key = "public_key";
  auto [auth_token, allocation] =
      account_manager.generate_account(public_key, index);

  // TODO(unknown): update this when we allow bigger allocations
  EXPECT_EQ(allocation.size(), 1);

  EXPECT_EQ(allocation[0], index);

  EXPECT_TRUE(account_manager.valid_index_access(auth_token, index));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index + 1));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index - 1));

  EXPECT_FALSE(account_manager.valid_index_access("", index));

  EXPECT_FALSE(account_manager.valid_index_access("public_key1", index));
  EXPECT_FALSE(account_manager.valid_index_access("public_ke", index));
}

TYPED_TEST(AccountManagerTest, RealPublicKey) {
  string db_address = "127.0.0.1";
  string db_password = "password";
  TypeParam account_manager(db_address, db_password);

  auto index = 1;

  // TODO(sualeh)
  // trunk-ignore(clang-tidy/cppcoreguidelines-avoid-c-arrays)
  char public_key_buffer[2];  // we should not use these in the future.
  // a real public key has non-utf8 characters
  const int outside_ascii_range = 255;
  public_key_buffer[0] = static_cast<char>(0);  // invalid character
  public_key_buffer[0] =
      static_cast<char>(outside_ascii_range);  // possibly invalid character
  string public_key;
  public_key.assign(public_key_buffer, sizeof(public_key_buffer));
  auto [auth_token, allocation] =
      account_manager.generate_account(public_key, index);

  // TODO(unknown): update this when we allow bigger allocations
  EXPECT_EQ(allocation.size(), 1);

  EXPECT_EQ(allocation[0], index);

  EXPECT_TRUE(account_manager.valid_index_access(auth_token, index));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index + 1));

  EXPECT_FALSE(account_manager.valid_index_access(auth_token, index - 1));

  EXPECT_FALSE(account_manager.valid_index_access("", index));

  EXPECT_FALSE(account_manager.valid_index_access("public_key1", index));
  EXPECT_FALSE(account_manager.valid_index_access("public_ke", index));
}

TYPED_TEST(AccountManagerTest, Indices) {
  string db_address = "127.0.0.1";
  string db_password = "password";
  TypeParam account_manager(db_address, db_password);

  const auto* public_key = "public_key";
  auto [auth_token, allocation] =
      account_manager.generate_account(public_key, 123);
  auto [auth_token2, allocation2] =
      account_manager.generate_account(public_key, 4324);

  auto pir_indices = account_manager.get_all_pir_indices();
  auto pir_indices_set = std::set<int>(pir_indices.begin(), pir_indices.end());
  // we don't assert anything about the size because if in Postgres, we will
  // have persisted many indices over the years EXPECT_EQ(pir_indices.size(),
  // 2);
  EXPECT_TRUE(pir_indices_set.contains(123));
  EXPECT_TRUE(pir_indices_set.contains(4324));
}