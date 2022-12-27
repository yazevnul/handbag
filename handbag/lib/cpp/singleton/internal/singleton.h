#pragma once

#include <array>
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#include <new>
#include <typeindex>
#include <typeinfo>
#include <utility>

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
    void* const res = memory.data();
    return res;
  }

  void init() noexcept {}
  void destroy() noexcept {}

 private:
  alignas(alignof(T)) static std::array<std::byte, sizeof(T)> memory;
};

template <typename T, typename Tag>
alignas(alignof(T))
    std::array<std::byte, sizeof(T)> StaticMemory<T, Tag>::memory = {};

template <typename T, typename Tag>
class DynamicMemory {
  struct Wrapper {
    alignas(alignof(T)) std::array<std::byte, sizeof(T)> memory = {};
  };

 public:
  ABSL_ATTRIBUTE_RETURNS_NONNULL void* get() const noexcept {
    void* const res = wrapper_->memory.data();
    return res;
  }

  void init() noexcept {
    wrapper_ = ::new (std::nothrow) Wrapper();
    if (wrapper_ == nullptr) {
      std::fprintf(
          stderr,
          "FATAL: Couldn't allocate memory for the singleton instance state\n");
      std::abort();
    }
  }

  void destroy() noexcept { ::delete wrapper_; }

 private:
  Wrapper* wrapper_ = nullptr;
};

constexpr bool IsSingletonDynamicallyAllocated(const size_t size) noexcept {
  constexpr size_t kLimit = 1024ULL * 1024;
  auto res = size > kLimit;
  return res;
}

template <typename T, typename Tag>
using Memory = std::conditional_t<IsSingletonDynamicallyAllocated(sizeof(T)),
                                  DynamicMemory<T, Tag>, StaticMemory<T, Tag>>;

template <bool Noexcept>
ABSL_ATTRIBUTE_RETURNS_NONNULL void* CreateInstance(
    std::pair<void*, void*> (*func)(), std::type_index key,
    int priority) noexcept(Noexcept);

template <typename T, typename Tag, int Priority,
          template <typename...> typename Traits>
class Singleton final {
  static constexpr bool kIsNothrowConstructible =
      noexcept(Traits<T, Tag>::Construct(std::declval<void*>()));

 public:
  static ABSL_ATTRIBUTE_RETURNS_NONNULL T* getInstance() noexcept(
      kIsNothrowConstructible) {
    static T* const instance = reinterpret_cast<T*>(
        singleton_internal::CreateInstance<kIsNothrowConstructible>(
            &CreateInstance, singleton_internal::Tag<T, Tag>::key(), Priority));
    return instance;
  }

 private:
  struct State {
    void (*destroy)(void*) = &DestroyInstance;
    Memory<T,
           singleton_internal::Tag<Tag, std::integral_constant<int, Priority>>>
        storage;
  };

  static_assert(std::is_standard_layout_v<State>);

  static std::pair<void*, void*> CreateInstance() noexcept(
      kIsNothrowConstructible) {
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

  static void DestroyInstance(void* const state) noexcept(
      std::is_nothrow_destructible_v<T>) ABSL_ATTRIBUTE_NONNULL(1) {
    auto* const typed = reinterpret_cast<State*>(state);

    const absl::Cleanup destroy_storage = [&] { typed->storage.destroy(); };

    auto* const instance = reinterpret_cast<T*>(typed->storage.get());
    std::destroy_at(instance);
  }
};

}  // namespace handbag::singleton_internal
