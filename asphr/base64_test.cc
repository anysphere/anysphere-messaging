
#include "base64.hpp"

#include <gtest/gtest.h>

TEST(Base64, EmptyString) {
  auto out = Base64::Encode("");
  EXPECT_EQ(out, "");
}