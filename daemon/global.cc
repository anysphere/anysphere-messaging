#include "global.hpp"

auto Global::kill() -> void {
  {
    const std::lock_guard<std::mutex> kill_l(kill_mtx);
    kill_ = true;
  }

  kill_cv.notify_all();
}

auto Global::wait_until_killed_or_seconds(int seconds) -> bool {
  std::unique_lock<std::mutex> kill_l(kill_mtx);

  if (kill_) {
    return true;
  }

  kill_cv.wait_for(kill_l, std::chrono::seconds(seconds),
                   [this] { return kill_; });

  kill_l.unlock();

  // kill only ever changes false -> true, so we are fine returning this here
  // even after unlocking
  return kill_;
}