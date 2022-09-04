//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include <cstdlib>  // std::exit
#include <format>
#include <iostream>  // std::cerr

#include "log.hpp"

int main(int argc, char** argv) {
  ASPHR_LOG_ERR("error.", key, "value");
  ASPHR_LOG_WARN("warning.", key, "value", key2, 2);
  ASPHR_LOG_INFO("info.", info1, "value1");
  ASPHR_LOG_DBG("debug.", c++ version, "c++20", key2, 2, key3, 3);
  return 0;
}