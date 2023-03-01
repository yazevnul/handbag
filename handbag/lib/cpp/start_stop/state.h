#pragma once

#include <atomic>

namespace handbag::start_stop {

class StoppableState {
 public:
  void SetStopping() noexcept;

  void SetStopped() noexcept;

  bool IsStopping() const noexcept;

  bool IsNotStopping() const noexcept;

  bool IsStopped() const noexcept;

  bool IsNotStopped() const noexcept;

  bool IsStoppingOrStopped() const noexcept;

  bool IsNotStoppingOrStopped() const noexcept;

 private:
  enum class EState { Unknown, Stopping, Stopped };

 private:
  std::atomic<EState> state_ = EState::Unknown;
};

// Implementation

void StoppableState::SetStopping() noexcept {
  state_.store(EState::Stopping, std::memory_order_release);
}

void StoppableState::SetStopped() noexcept {
  state_.store(EState::Stopped, std::memory_order_release);
}

bool StoppableState::IsStopping() const noexcept {
  const auto state = state_.load(std::memory_order_acquire);
  auto res = state == EState::Stopping;
  return res;
}

bool StoppableState::IsNotStopping() const noexcept { return !IsStopping(); }

bool StoppableState::IsStopped() const noexcept {
  const auto state = state_.load(std::memory_order_acquire);
  auto res = state == EState::Stopped;
  return res;
}

bool StoppableState::IsNotStopped() const noexcept { return !IsStopped(); }

bool StoppableState::IsStoppingOrStopped() const noexcept {
  const auto state = state_.load(std::memory_order_acquire);
  auto res = state != EState::Unknown;
  return res;
}

bool StoppableState::IsNotStoppingOrStopped() const noexcept {
  return !IsStoppingOrStopped();
}

}  // namespace handbag::start_stop
