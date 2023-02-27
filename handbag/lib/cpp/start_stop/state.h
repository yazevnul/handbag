#pragma once

#include <atomic>

namespace handbag::start_stop {

class StoppableState {
enum class EState {
    Unknown,
    Stopping,
    Stopped
};

public:
    void SetStopping() noexcept {
        state_.store(EState::Stopping, std::memory_order_release);
    }

    void SetStopped() noexcept {
        state_.store(EState::Stopped, std::memory_order_release);
    }

    bool IsStopping() const noexcept {
        const auto state = state_.load(std::memory_order_acquire);
        auto res = state == EState::Stopping;
        return res;
    }

    bool IsNotStopping() const noexcept {
        return !IsStopping();
    }

    bool IsStopped() const noexcept {
        const auto state = state_.load(std::memory_order_acquire);
        auto res = state == EState::Stopped;
        return res;
    }

    bool IsNotStopped() const noexcept {
        return !IsStopped();
    }

    bool IsStoppingOrStopped() const noexcept {
        const auto state = state_.load(std::memory_order_acquire);
        auto res = state != EState::Unknown;
        return res;
    }

    bool IsNotStoppingOrStopped() const noexcept {
        return !IsStoppingOrStopped();
    }

private:
  std::atomic<EState> state_ = EState::Unknown;
};

}
