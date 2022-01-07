#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "cli/scheduler.h"

namespace as_cli {

/**
 * @brief The LoopScheduler is a simple thread-safe scheduler
 *
 */
class AnysphereScheduler : public cli::Scheduler {
 public:
  AnysphereScheduler() = default;
  ~AnysphereScheduler() override { Stop(); }

  // non copyable
  AnysphereScheduler(const AnysphereScheduler&) = delete;
  AnysphereScheduler& operator=(const AnysphereScheduler&) = delete;

  void Stop();
  bool ExecOne();
  void Post(const std::function<void()>& f) override;
  void DisplayMessage();

  void Run() {
    while (ExecOne()) {
    };
  }

 private:
  std::queue<std::function<void()>> tasks;
  bool running{true};
  std::mutex mtx;
  std::condition_variable cv;
};

}  // namespace as_cli
