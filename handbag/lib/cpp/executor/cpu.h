#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>

#include "lib/cpp/executor/executor.h"
#include "lib/cpp/start_stop/start_stop.h"

namespace handbag::executor {

struct CpuExecutorParams {
  std::optional<std::string> name;
  std::optional<size_t> thread_count;
  std::optional<size_t> queue_capacity;
};

class CpuExecutor final : public IExecutor, public start_stop::IStoppable {
  class NotPubliclyConstructible {};

 public:
  CpuExecutor() = delete;
  CpuExecutor(const CpuExecutor&) = delete;
  CpuExecutor(CpuExecutor&&) = delete;
  CpuExecutor& operator=(const CpuExecutor&) = delete;
  CpuExecutor& operator=(CpuExecutor&&) = delete;

  CpuExecutor(NotPubliclyConstructible /*npc*/,
              const CpuExecutorParams& params);
  ~CpuExecutor() override;

  std::unique_ptr<CpuExecutor> create(const CpuExecutorParams& params);

  void Add(absl::AnyInvocable<void() &&> task) noexcept override;
  bool TryAdd(absl::AnyInvocable<void() &&>&& task) noexcept override;

  std::future<void> stop() override;

 private:
  class Impl;
  // TODO(kostya): make it an inline impl
  std::unique_ptr<Impl> i_;
};

}  // namespace handbag::executor
