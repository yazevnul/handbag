#include "lib/cpp/executor/cpu.h"

#include <future>
#include <thread>

#include "lib/cpp/repr/repr.h"
#include "lib/cpp/start_stop/state.h"

namespace handbag::executor {

class CpuExecutor::Impl final : public IExecutor,
                                public start_stop::IStoppable,
                                public IRepr,
                                public start_stop::StoppableState {
 public:
  Impl(const CpuExecutorParams& params)
      : params_{.name = params.name,
                .thread_count = params.thread_count > 0
                                    ? params.thread_count
                                    : std::thread::hardware_concurrency(),
                .queue_capacity = params.queue_capacity} {}

  void Add(absl::AnyInvocable<void() &&> task) noexcept override;
  bool TryAdd(absl::AnyInvocable<void() &&>&& task) noexcept override;

  std::future<void> stop() override {
    SetStopping();
      auto res = std::async(
  }

  std::string GetRepr() const noexcept override {
      return Repr::create("CpuExecutor")
          .field("name", params_.name)
          .field("thread_count", params_.thread_count.value())
          .field("queue_capacity", params_.queue_capacity.value())
          .end();
  }

 private:
  CpuExecutorParams params_;
};

}  // namespace handbag::executor
