//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#include <cstdlib>   // std::exit
#include <iostream>  // std::cerr

#include "assert.hpp"

void foo(int i);
void bar(int i);

void bar(int i) {
  // boost::array<int, 5> a = {{101, 100, 123, 23, 32}};
  if (i >= 0) {
    foo(i);
  } else {
    std::exit(1);
  }
}

int counter_glob;
int counter() { return counter_glob++; }

void foo(int i) {
  std::cout << "here " << i << std::endl;
  //   ASPHR_ASSERT_EQ(counter(), 3);
  ASPHR_ASSERT_MSG(i >= 1, "value of i: " << i);
  ASPHR_ASSERT(i >= 1);
  bar(--i);
}

int main(int argc, char** argv) {
  counter_glob = 3;
  //   ASPHR_ASSERT_EQ(1, 0);
  foo(3);
  std::cout << "Hello, world! !" << std::endl;
  assert(false);
  return 0;
}