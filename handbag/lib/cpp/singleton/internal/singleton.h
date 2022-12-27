#pragma once

#include <typeinfo>
#include <typeindex>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <cstdio>
#include <cinttypes>
#include <new>
#include <utility>
#include <typeindex>
#include <exception>

#include "absl/base/attributes.h"
#include "absl/base/attributes.h"
#include "absl/cleanup/cleanup.h"

namespace handbag::singleton_internal {

template <typename... Args>
struct Tag {
    static std::type_index key() noexcept {
        std::type_index res = typeid(Tag<Args...>);
        return res;
    }
};

template <typename T, typename Tag>
class StaticMemory {
public:
    ABSL_ATTRIBUTE_RETURNS_NONNULL void* get() const noexcept {
        void* const res = kStorage;
        return res;
    }

    void init() noexcept {}
    void destroy() noexcept {}

private:
    alignas(alignof(T)) static std::byte kStorage[sizeof(T)];
};

template <typename T, typename Tag>
class DynamicMemory {
    struct Storage {
        alignas(alignof(T)) std::byte storage[sizeof(T)];
    };
public:
    ABSL_ATTRIBUTE_RETURNS_NONNULL void* get() const noexcept {
        void* const res = storage_->storage;
        return res;
    }

    void init() noexcept {
        storage_ = ::new(std::nothrow) Storage();
        if (storage_ == nullptr) {
            std::fprintf(stderr, "FATAL: Couldn't allocate memory for the singleton instance state\n");
            std::abort();
        }
    }

    void destroy() noexcept {
        ::delete storage_;
    }

private:
    Storage* storage_ = nullptr;
};

constexpr bool IsSingletonDynamicallyAllocated(const size_t size) noexcept {
    constexpr size_t kLimit = 1024ULL * 1024;
    auto res = size > kLimit;
    return res;
}

template <typename T, typename Tag>
using Memory = std::conditional_t<
    IsSingletonDynamicallyAllocated(sizeof(T)),
    DynamicMemory<T, Tag>,
    StaticMemory<T, Tag>>;

template <bool Noexcept>
ABSL_ATTRIBUTE_RETURNS_NONNULL void* CreateInstance(std::pair<void*, void*>(*func)(), std::type_index key, int priority) noexcept(Noexcept);

template <typename T, typename Tag, int Priority, template <typename...> typename Traits>
class Singleton final {
public:
    static ABSL_ATTRIBUTE_RETURNS_NONNULL T* getInstance() noexcept(std::is_nothrow_invocable_v<typename Traits<T, Tag>::Construct>) {
        static T* const instance = singleton_internal::CreateInstance<std::is_nothrow_invocable_v<typename Traits<T, Tag>::Construct>>(&CreateInstance, singleton_internal::Tag<T, Tag>::key(), Priority);
        return instance;
    }

private:
    struct State {
        void (*destroy)(void*) = &DestroyInstance;
        singleton_internal::Memory<T, singleton_internal::Tag<Tag, std::integral_constant<int, Priority>>> storage;
    };

    static_assert(std::is_standard_layout_v<State>);

    static std::pair<void*, void*> CreateInstance() noexcept(std::is_nothrow_invocable_v<typename Traits<T, Tag>::Construct>) {
        static State state;

        state.storage.init();

        const absl::Cleanup destroy_storage_on_exception = [&]() mutable {
            if (std::uncaught_exceptions() > 0) {
                state.storage.destroy();
            }
        };

        Traits<T, Tag>::Construct(state.storage.get());

        return {&state, state.storage.get()};
    }

    static void DestroyInstance(void* const state) noexcept(std::is_nothrow_invocable_v<typename Traits<T, Tag>::Destroy, void*>) ABSL_ATTRIBUTE_NONNULL(1) {
        auto* const typed = reinterpret_cast<State*>(state);

        const absl::Cleanup destroy_storage = [&] {
            typed->storage.destroy();
        };

        Traits<T, Tag>::Destroy(typed->storage.get());
    }
};

}
