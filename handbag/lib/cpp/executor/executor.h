#pragma once

#include <future>
#include <optional>
#include <type_traits>

#include "absl/functional/any_invocable.h"
#include "lib/cpp/executor/internal/executor.h"

namespace handbag {

struct IExecutor {
  virtual ~IExecutor() = default;

  void Add(absl::AnyInvocable<void() &&> task) noexcept;
  bool TryAdd(absl::AnyInvocable<void() &&>&& task) noexcept;
};

template <typename Invocable, typename... Args>
std::future<
    std::invoke_result_t<std::decay_t<Invocable>, std::decay_t<Args>...>>
AddTo(IExecutor& executor, Invocable&& invocable, Args&&... args);

template <typename Invocable, typename... Args>
std::optional<std::future<
    std::invoke_result_t<std::decay_t<Invocable>, std::decay_t<Args>...>>>
TryAddTo(IExecutor& executor, Invocable&& invocable, Args&&... args);

/// Impl

template <typename Invocable, typename... Args>
std::future<
    std::invoke_result_t<std::decay_t<Invocable>, std::decay_t<Args>...>>
AddTo(IExecutor& executor, Invocable&& invocable, Args&&... args) {
  auto [res, closure] = internal_executor::CreateTask(
      std::forward<Invocable>(invocable), std::forward<Args>(args)...);
  executor.Add(std::move(closure));
  return res;
}

template <typename Invocable, typename... Args>
std::optional<std::future<
    std::invoke_result_t<std::decay_t<Invocable>, std::decay_t<Args>...>>>
TryAddTo(IExecutor& executor, Invocable&& invocable, Args&&... args) {
  auto [res, closure] = internal_executor::CreateTask(
      std::forward<Invocable>(invocable), std::forward<Args>(args)...);
  if (executor.TryAdd(std::move(closure))) {
    return res;
  } else {
    // TODO(kostya): move the contents of the closure back into `args`.
  }

  return std::nullopt;
}

}  // namespace handbag
