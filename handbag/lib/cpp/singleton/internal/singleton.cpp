#include "lib/cpp/singleton/internal/singleton.h"

#include <atomic>
#include <cstdlib>
#include <cstdint>
#include <thread>
#include <cstdio>
#include <map>
#include <typeindex>
#include <mutex>
#include <utility>
#include <queue>
#include <tuple>

#include "absl/base/attributes.h"
#include "absl/cleanup/cleanup.h"
#include "absl/base/optimization.h"

namespace handbag::singleton_internal {
namespace {

struct SingletonStatePrefix {
    void (*destroy)(void*) = nullptr;
};

class SingletonVault {
    struct Entry {
        std::mutex mutex;
        int priority = 0;
        void* state = nullptr;
        void* instance = nullptr;
    };

public:
    template <bool Noexcept>
    ABSL_ATTRIBUTE_RETURNS_NONNULL void* CreateInstance(std::pair<void*, void*>(*const func)(), const std::type_index key, const int priority) noexcept(Noexcept) {
        auto* const entry = GetEntry(key, priority);

        const std::unique_lock lock(entry->mutex);
        if (entry->instance == nullptr) {
            std::tie(entry->state, entry->instance) = func();
        }

        return entry->instance;
    }

    void DestroyInstances() {
        const std::unique_lock lock(mutex_);
        for (const auto& [priority, entries] : ordered_) {
            (void)priority;
            for (auto* const entry : entries) {
                const std::unique_lock entry_lock(entry->mutex);
                if (entry->state == nullptr) {
                    continue;
                }

                auto* const destroy = reinterpret_cast<SingletonStatePrefix*>(entry->state)->destroy;
                destroy(entry->state);
            }
        }

        entries_.clear();
        ordered_.clear();
    }

private:
    ABSL_ATTRIBUTE_RETURNS_NONNULL Entry* GetEntry(const std::type_index key, const int priority) {
        const std::unique_lock lock(mutex_);
        auto it = entries_.find(key);
        if (it == entries_.end()) {
            it = entries_.emplace(key, std::make_unique<Entry>()).first;
            it->second->priority = priority;
            ordered_[priority].push_back(it->second.get());
        } else {
            if (ABSL_PREDICT_FALSE(it->second->priority != priority)) {
                std::fprintf(stderr, "FATAL: got singletons of the same type and tag, but with different priorities; existing=%d, new=%d", it->second->priority, priority);
                std::abort();
            }
        }

        auto* res = it->second.get();

        return res;
    }

private:
    std::mutex mutex_;
    std::map<std::type_index, std::unique_ptr<Entry>> entries_;
    std::map<int, std::vector<Entry*>> ordered_;
};

constexpr int kUninitialized = 0;
constexpr int kInitializing = 1;
constexpr int kInitialized = 2;
constexpr int kDestroying = 3;
constexpr int kDestroyed = 3;

static std::atomic<int> vault_state = kUninitialized;
alignas(alignof(SingletonVault)) static std::byte storage[sizeof(SingletonVault)];

void DestroySingletonVault() {
    if (auto state = kInitialized; ABSL_PREDICT_FALSE(!vault_state.compare_exchange_strong(state, kDestroying))) {
        if (state == kUninitialized) {
            std::fprintf(stderr, "FATAL: Trying to destroy uninitialized singleton vault.\n");
        } else if (state == kInitializing) {
            std::fprintf(stderr, "FATAL: Trying to destroy singleton vault while it's being initialized.\n");
        } else if (state != kDestroying) {
            std::fprintf(stderr, "FATAL: Trying to destroy singleton vault while it's being destroyed.\n");
        } else if (state == kDestroyed) {
            std::fprintf(stderr, "FATAL: Trying to destroy destroyed singleton vault.\n");
        }

        std::abort();
    }

    const absl::Cleanup move_to_destroyed_state = [] { vault_state.store(kDestroyed, std::memory_order_release); };

    auto* const vault = reinterpret_cast<SingletonVault*>(storage);
    vault->DestroyInstances();
    std::destroy_at(vault);
}

ABSL_ATTRIBUTE_RETURNS_NONNULL SingletonVault* GetSingletonVault() noexcept {
    if (const auto state = vault_state.load(std::memory_order_acquire); state == kInitialized) {
        return reinterpret_cast<SingletonVault*>(storage);
    } else if (ABSL_PREDICT_FALSE(state == kDestroying || state == kDestroyed)) {
        if (state == kDestroying) {
            std::fprintf(stderr, "FATAL: Trying to access singleton vault while it's being destroyed.\n");
        } else if (state == kDestroyed) {
            std::fprintf(stderr, "FATAL: Trying to access a destroyed singleton vault.\n");
        }

        std::abort();
    }

    if (auto state = kUninitialized; !vault_state.compare_exchange_strong(state, kInitializing)) {
        for (;;) {
            state = vault_state.load(std::memory_order_acquire);
            if (state == kInitializing) {
                std::this_thread::yield();
            } else if (state == kInitialized) {
                return reinterpret_cast<SingletonVault*>(storage);
            } else if (ABSL_PREDICT_FALSE(state == kUninitialized || state == kDestroying || state == kDestroyed)) {
                if (state == kUninitialized) {
                    std::fprintf(stderr, "FATAL: Waiting for uninitialized singleton vault initialization.\n");
                } else if (state == kDestroying) {
                    std::fprintf(stderr, "FATAL: Trying to access singleton vault while it's being destroyed.\n");
                } else if (state == kDestroyed) {
                    std::fprintf(stderr, "FATAL: Trying to access a destroyed singleton vault.\n");
                }

                std::abort();
            }
        }
    }

    auto* const res = ::new (storage) SingletonVault();
    std::atexit(&DestroySingletonVault);
    vault_state.store(kInitialized);

    return res;
}

}

template <bool Noexcept>
void* CreateInstance(std::pair<void*, void*>(*const func)(), const std::type_index key, const int priority) noexcept(Noexcept) {
    auto res = GetSingletonVault()->CreateInstance<Noexcept>(func, key, priority);
    return res;
}

template void* CreateInstance<false>(std::pair<void*, void*>(*func)(), std::type_index key, int priority);
template void* CreateInstance<true>(std::pair<void*, void*>(*func)(), std::type_index key, int priority) noexcept;

}
