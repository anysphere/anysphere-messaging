//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include "main.hpp"

#include <iostream>

#include "daemon-rs/main.rs.h"

int main_cc() {
  std::cout << "hi" << std::endl;
  std::cout << db::get_friend_uid(4) << std::endl;
  std::cout << db::get_friend(4).uid << std::endl;
  return 0;
}
