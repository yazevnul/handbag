#pragma once

#include <exception>
#include <functional>
#include <future>
#include <tuple>
#include <utility>

#include "absl/functional/any_invocable.h"

namespace handbag::internal_executor {

template <typename Invocable, typename... Args>
std::pair<std::future<std::invoke_result_t<std::decay_t<Invocable>,
                                           std::decay_t<Args>...>>,
          absl::AnyInvocable<void() &&>>
CreateTask(Invocable&& invocable, Args&&... args) {
  using Result =
      std::invoke_result_t<std::decay_t<Invocable>, std::decay_t<Args>...>;

  std::promise<Result> promise;
  auto future = promise.get_future();
  auto res = std::make_pair<std::future<Result>, absl::AnyInvocable<void() &&>>(
      std::move(future),
      [closure = std::make_tuple(std::move(promise),
                                 std::forward<Invocable>(invocable),
                                 std::forward<Args>(args)...)]() mutable {
        std::apply(
            [](std::promise<Result>&& promise, Invocable&& invocable,
               Args&&... args) {
              try {
                if constexpr (std::is_same_v<Result, void>) {
                  std::invoke(std::forward<Invocable>(invocable),
                              std::forward<Args>(args)...);
                  promise.set_value();
                } else {
                  auto res = std::invoke(std::forward<Invocable>(invocable),
                                         std::forward<Args>(args)...);
                  promise.set_value(std::move(res));
                }
              } catch (...) {
                auto eptr = std::current_exception();
                promise.set_exception(std::move(eptr));
              }
            },
            std::move(closure));
      });
  return res;
}

}  // namespace handbag::internal_executor
