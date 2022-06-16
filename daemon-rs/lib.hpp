#pragma once

#include <iostream>

#include "daemon-rs/main.rs.h"

using std::cout;
using std::endl;

int lib() {
  cout << "cool thing from rust: " << f() << endl;
  auto x = g();
  cout << "uid friend: " << x.uid << endl;
  return 1;
}