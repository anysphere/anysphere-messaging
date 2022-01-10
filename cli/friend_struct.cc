#include "as_cli.hpp"

void Friend::add() {
  set_time();
  // write_friend_to_file(file_address_, name_, write_index_, read_index_,
  //  shared_key_, time_);
  clear();
}

bool Friend::complete() const {
  return !name_is_empty() && !shared_key_is_empty() && write_index_ != -1 &&
         read_index_ != -1;
}