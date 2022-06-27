#pragma once

#include "rust/cxx.h"

// no return code. instead, to exit with a return code, explicitly call
// exit(code). if this function returns normally, the process will be exited
// with status code 0.
auto main_cc(rust::Vec<rust::String> args) -> void;