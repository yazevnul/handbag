#include "lib/cpp/executor/cpu.h"

#include <exception>
#include <future>
#include <queue>
#include <thread>

#include "absl/base/optimization.h"
#include "absl/base/thread_annotations.h"
#include "absl/log/log.h"
#include "absl/synchronization/mutex.h"
#include "lib/cpp/repr/repr.h"
#include "lib/cpp/start_stop/state.h"

namespace handbag::executor {

namespace {
struct Params {
  std::string name;
  size_t thread_count = 0;
  size_t queue_capacity = 0;
};
}  // namespace

class CpuExecutor::Impl final : public IExecutor,
                                public start_stop::IStoppable,
                                public IRepr,
                                public start_stop::StoppableState {
 public:
  Impl(const CpuExecutorParams& params)
      : params_{
            .name = params.name.has_value() ? params.name.value() : "CpuExec",
            .thread_count = params.thread_count > 0
                                ? params.thread_count.value()
                                : std::thread::hardware_concurrency(),
            .queue_capacity = params.queue_capacity.has_value()
                                  ? params.queue_capacity.value()
                                  : std::numeric_limits<size_t>::max()} {
    workers_.reserve(params_.thread_count);
    for (size_t i = 0; i < params_.thread_count; ++i) {
      auto worker = std::async(std::launch::async, &Impl::WorkerTask, this);
      workers_.push_back(std::move(worker));
    }
  }

  void Add(absl::AnyInvocable<void() &&> task) noexcept override {
    const absl::MutexLock lock(&mutex_);
    if (!HasSpareCapacity()) {
      mutex_.Await(absl::Condition(this, &Impl::HasSpareCapacity));
    }

    tasks_.push(std::move(task));
  }

  bool TryAdd(absl::AnyInvocable<void() &&>&& task) noexcept override {
    const absl::MutexLock lock(&mutex_);
    if (!HasSpareCapacity()) {
      return false;
    }

    tasks_.push(std::move(task));

    return true;
  }

  std::future<void> stop() override {
    SetStopping();

    const absl::MutexLock lock(&mutex_);
    for (size_t i = 0; i < params_.thread_count; ++i) {
      tasks_.push(nullptr);
    }

    auto res = std::async(std::launch::async, [this]() noexcept {
      for (auto& worker : workers_) {
        try {
          worker.get();
        } catch (...) {
          auto eptr = std::current_exception();
          std::string message;
          try {
            std::rethrow_exception(eptr);
          } catch (const std::exception& exc) {
            message = exc.what();
          }

          LOG(ERROR) << *this << "; what() = " << message;
        }
      }

      SetStopped();
    });

    return res;
  }

  std::string GetRepr() const noexcept override {
    return Repr::create("CpuExecutor")
        .field("name", params_.name)
        .field("thread_count", params_.thread_count)
        .field("queue_capacity", params_.queue_capacity)
        .end();
  }

 private:
  bool HasTasksToRun() const noexcept ABSL_SHARED_LOCKS_REQUIRED(mutex_) {
    auto res = tasks_.size() > 0;
    return res;
  }

  bool HasSpareCapacity() const noexcept ABSL_SHARED_LOCKS_REQUIRED(mutex_) {
    auto res = tasks_.size() < params_.queue_capacity;
    return res;
  }

  void WorkerTask() {
    for (;;) {
      const absl::MutexLock lock(&mutex_);
      mutex_.Await(absl::Condition(this, &Impl::HasTasksToRun));

      auto task = std::move(tasks_.front());
      tasks_.pop();

      if (ABSL_PREDICT_FALSE(!task)) {
        break;
      }

      try {
        std::move(task)();
      } catch (...) {
        auto eptr = std::current_exception();
        std::string message;
        try {
          std::rethrow_exception(eptr);
        } catch (const std::exception& exc) {
          message = exc.what();
        }

        LOG(ERROR) << *this << "; what() = " << message;
      }
    }
  }

 private:
  Params params_;

  absl::Mutex mutex_;
  std::queue<absl::AnyInvocable<void() &&>> tasks_ ABSL_GUARDED_BY(mutex_);
  std::vector<std::future<void>> workers_;
};

CpuExecutor::CpuExecutor(NotPubliclyConstructible /*npc*/,
                         const CpuExecutorParams& params)
    : i_(std::make_unique<Impl>(params)) {}

CpuExecutor::~CpuExecutor() = default;

std::unique_ptr<CpuExecutor> CpuExecutor::create(
    const CpuExecutorParams& params) {
  auto res = std::make_unique<CpuExecutor>(NotPubliclyConstructible(), params);
  return res;
}

void CpuExecutor::Add(absl::AnyInvocable<void() &&> task) noexcept {
  i_->Add(std::move(task));
}

bool CpuExecutor::TryAdd(absl::AnyInvocable<void() &&>&& task) noexcept {
  auto res = i_->TryAdd(std::move(task));
  return res;
}

std::future<void> CpuExecutor::stop() {
  auto res = i_->stop();
  return res;
}

}  // namespace handbag::executor
