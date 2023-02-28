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
class StaticStorage {
 public:
  ABSL_ATTRIBUTE_RETURNS_NONNULL void* get() noexcept {
    void* const res = memory.data();
    return res;
  }

  void allocate() noexcept { memory.fill(std::byte{}); }

  void deallocate() noexcept { memory.fill(std::byte{}); }

 private:
  alignas(alignof(T)) std::array<std::byte, sizeof(T)> memory;
};

template <typename T, typename Tag>
class DynamicStorage {
  struct Wrapper {
    alignas(alignof(T)) std::array<std::byte, sizeof(T)> memory = {};
  };

 public:
  ABSL_ATTRIBUTE_RETURNS_NONNULL void* get() noexcept {
    void* const res = wrapper_->memory.data();
    return res;
  }

  void allocate() noexcept {
    wrapper_ = ::new (std::nothrow) Wrapper();
    if (wrapper_ == nullptr) {
      std::fprintf(
          stderr,
          "FATAL: Couldn't allocate memory for the singleton instance state\n");
      std::abort();
    }
  }

  void deallocate() noexcept {
    wrapper_->memory.fill(std::byte{});
    ::delete wrapper_;
  }

 private:
  Wrapper* wrapper_;
};

constexpr bool IsSingletonDynamicallyAllocated(const size_t size) noexcept {
  constexpr size_t kLimit = 1024ULL * 1024;
  auto res = size > kLimit;
  return res;
}

template <typename T, typename Tag>
using Storage =
    std::conditional_t<IsSingletonDynamicallyAllocated(sizeof(T)),
                       DynamicStorage<T, Tag>, StaticStorage<T, Tag>>;

template <typename Traits>
constexpr bool IsNoexceptConstructible() noexcept {
  auto res = noexcept(Traits::Construct(std::declval<void*>()));
  return res;
}

template <bool CreateFnNoexcept>
ABSL_ATTRIBUTE_RETURNS_NONNULL void* CreateInstance(
    void* (*create_fn)(), void (*destroy_fn)(void*), std::type_index key,
    int priority) noexcept(CreateFnNoexcept);

template <typename T, typename Tag, int Priority,
          template <typename...> typename Traits>
class Singleton final {
  static constexpr bool kIsNothrowConstructible =
      IsNoexceptConstructible<Traits<T, Tag>>();
  using Storage = singleton_internal::Storage<T, Tag>;
  using SingletonTag = singleton_internal::Tag<T, Tag>;

 public:
  static ABSL_ATTRIBUTE_RETURNS_NONNULL T* getInstance() noexcept(
      kIsNothrowConstructible) {
    static auto* const instance = reinterpret_cast<T*>(
        reinterpret_cast<Storage*>(
            singleton_internal::CreateInstance<kIsNothrowConstructible>(
                &CreateInstance, &DestroyInstance, SingletonTag::key(),
                Priority))
            ->get());
    return instance;
  }

 private:
  static void* CreateInstance() noexcept(kIsNothrowConstructible) {
    static_assert(std::is_trivially_constructible_v<Storage>);
    static_assert(std::is_trivially_destructible_v<Storage>);
    static Storage storage;

    storage.allocate();

    const absl::Cleanup deallocate_on_exception = []() noexcept {
      if (std::uncaught_exceptions() > 0) {
        storage.deallocate();
      }
    };

    Traits<T, Tag>::Construct(storage.get());

    return &storage;
  }

  static void DestroyInstance(void* const untyped_storage) noexcept(
      std::is_nothrow_destructible_v<T>) ABSL_ATTRIBUTE_NONNULL(1) {
    auto& storage = *reinterpret_cast<Storage*>(untyped_storage);

    const absl::Cleanup deallocate = [&]() noexcept { storage.deallocate(); };

    auto* const instance = reinterpret_cast<T*>(storage.get());
    std::destroy_at(instance);
  }
};

}  // namespace handbag::singleton_internal
