#include "lib/cpp/singleton/internal/singleton.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <typeindex>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/base/optimization.h"
#include "absl/cleanup/cleanup.h"

namespace handbag::singleton_internal {
namespace {

enum EState : int {
  Uninitialized,
  Initializing,
  Initialized,
  Destroying,
  Destroyed
};

static std::atomic<EState> vault_state = EState::Uninitialized;

class SingletonVault {
  struct Entry {
    std::mutex mutex;
    int priority = 0;
    void* storage = nullptr;
    void (*destroy_fn)(void*) = nullptr;
  };

 public:
  template <bool CreateFnNoexcept>
  ABSL_ATTRIBUTE_RETURNS_NONNULL void* CreateInstance(
      void* (*const create_fn)(), void (*const destroy_fn)(void*),
      const std::type_index key,
      const int priority) noexcept(CreateFnNoexcept) {
    auto& entry = GetEntry(key, priority);

    int entry_priority = 0;
    void* entry_storage = nullptr;
    {
      const std::unique_lock lock(entry.mutex);
      if (entry.storage == nullptr) {
        entry.storage = create_fn();
        entry.destroy_fn = destroy_fn;
        entry.priority = priority;
      }

      entry_priority = entry.priority;
      entry_storage = entry.storage;
    }

    if (ABSL_PREDICT_FALSE(entry_priority != priority)) {
      std::fprintf(
          stderr,
          "FATAL: Singleton of the same type and with the same tag already "
          "exists, but priority is different; existing=%d, requested=%d",
          entry_priority, priority);
      std::abort();
    }

    return entry_storage;
  }

  void DestroyInstances() noexcept(false) {
    const std::unique_lock lock(mutex_);
    for (const auto& [priority, entries] : ordered_) {
      (void)priority;
      for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        auto& entry = **it;
        const std::unique_lock entry_lock(entry.mutex);
        if (entry.storage == nullptr) {
          continue;
        }

        entry.destroy_fn(entry.storage);
      }
    }

    ordered_.clear();
    entries_.clear();
  }

 private:
  Entry& GetEntry(const std::type_index key, const int priority) {
    const std::unique_lock lock(mutex_);
    auto& res =
        entries_
            .emplace(std::piecewise_construct, std::forward_as_tuple(key),
                     std::forward_as_tuple())
            .first->second;
    ordered_[priority].push_back(&res);
    return res;
  }

 private:
  std::mutex mutex_;
  std::map<std::type_index, Entry> entries_;
  std::map<int, std::vector<Entry*>> ordered_;
};

alignas(alignof(SingletonVault)) static std::array<
    std::byte, sizeof(SingletonVault)> vault_memory = {};

void DestroySingletonVault() {
  if (auto state = EState::Initialized; ABSL_PREDICT_FALSE(
          !vault_state.compare_exchange_strong(state, EState::Destroying))) {
    if (state == EState::Uninitialized) {
      std::fprintf(stderr,
                   "FATAL: Trying to destroy uninitialized singleton vault.\n");
    } else if (state == EState::Initializing) {
      std::fprintf(stderr,
                   "FATAL: Trying to destroy singleton vault while it's being "
                   "initialized.\n");
    } else if (state != EState::Destroying) {
      std::fprintf(stderr,
                   "FATAL: Trying to destroy singleton vault while it's being "
                   "destroyed.\n");
    } else if (state == EState::Destroyed) {
      std::fprintf(stderr,
                   "FATAL: Trying to destroy destroyed singleton vault.\n");
    }

    std::abort();
  }

  const absl::Cleanup move_to_destroyed_state = []() noexcept {
    vault_state.store(EState::Destroyed, std::memory_order_release);
  };

  auto* const vault = reinterpret_cast<SingletonVault*>(vault_memory.data());
  vault->DestroyInstances();
  std::destroy_at(vault);
}

ABSL_ATTRIBUTE_RETURNS_NONNULL SingletonVault* GetSingletonVault() noexcept {
  if (const auto state = vault_state.load(std::memory_order_acquire);
      state == EState::Initialized) {
    return reinterpret_cast<SingletonVault*>(vault_memory.data());
  } else if (ABSL_PREDICT_FALSE(state == EState::Destroying ||
                                state == EState::Destroyed)) {
    if (state == EState::Destroying) {
      std::fprintf(stderr,
                   "FATAL: Trying to access singleton vault while it's being "
                   "destroyed.\n");
    } else if (state == EState::Destroyed) {
      std::fprintf(stderr,
                   "FATAL: Trying to access a destroyed singleton vault.\n");
    }

    std::abort();
  }

  if (auto state = EState::Uninitialized;
      !vault_state.compare_exchange_strong(state, EState::Initializing)) {
    for (;;) {
      state = vault_state.load(std::memory_order_acquire);
      if (state == EState::Initializing) {
        std::this_thread::yield();
      } else if (state == EState::Initialized) {
        return reinterpret_cast<SingletonVault*>(vault_memory.data());
      } else if (ABSL_PREDICT_FALSE(state == EState::Uninitialized ||
                                    state == EState::Destroying ||
                                    state == EState::Destroyed)) {
        if (state == EState::Uninitialized) {
          std::fprintf(stderr,
                       "FATAL: Waiting for uninitialized singleton vault "
                       "initialization.\n");
        } else if (state == EState::Destroying) {
          std::fprintf(stderr,
                       "FATAL: Trying to access singleton vault while it's "
                       "being destroyed.\n");
        } else if (state == EState::Destroyed) {
          std::fprintf(
              stderr, "FATAL: Trying to access a destroyed singleton vault.\n");
        }

        std::abort();
      }
    }
  }

  auto* const res = ::new (vault_memory.data()) SingletonVault();
  std::atexit(&DestroySingletonVault);
  vault_state.store(EState::Initialized);

  return res;
}

}  // namespace

template <bool CreateFnNoexcept>
void* CreateInstance(void* (*const create_fn)(),
                     void (*const destroy_fn)(void*), const std::type_index key,
                     const int priority) noexcept(CreateFnNoexcept) {
  auto res = GetSingletonVault()->CreateInstance<CreateFnNoexcept>(
      create_fn, destroy_fn, key, priority);
  return res;
}

template void* CreateInstance<false>(void* (*create_fn)(),
                                     void (*destroy_fn)(void*),
                                     std::type_index key, int priority);
template void* CreateInstance<true>(void* (*create_fn)(),
                                    void (*destroy_fn)(void*),
                                    std::type_index key, int priority) noexcept;

}  // namespace handbag::singleton_internal
