#pragma once

#include "absl/functional/any_invocable.h"

namespace handbag {

struct IExecutor {
  virtual ~IExecutor() = default;

  void Add(absl::AnyInvocable<void()> task) noexcept;
  bool TryAdd(absl::AnyInvocable<void()>&& task) noexcept;
};

}  // namespace handbag
