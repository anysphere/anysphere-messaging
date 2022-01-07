#include "as_scheduler.hpp"

namespace as_cli {

void AnysphereScheduler::Stop() {
  std::lock_guard<std::mutex> lck(mtx);
  running = false;
  cv.notify_all();
}

void AnysphereScheduler::Post(const std::function<void()>& f) {
  std::lock_guard<std::mutex> lck(mtx);
  tasks.push(f);
  cv.notify_all();
}

bool AnysphereScheduler::ExecOne() {
  std::function<void()> task;
  {
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [this]() { return !running || !tasks.empty(); });
    if (!running) return false;
    task = tasks.front();
    tasks.pop();
  }

  if (task) task();

  return true;
}

}  // namespace as_cli