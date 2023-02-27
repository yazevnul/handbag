#pragma once

#include <memory>
#include <type_traits>

#include "lib/cpp/singleton/internal/singleton.h"
#include "lib/cpp/singleton/fwd.h"

namespace handbag {

template <typename T, typename Tag>
struct SingletonTraits {
  static void Construct(void* const ptr) noexcept(
      std::is_nothrow_default_constructible_v<T>) {
    auto* const typed = ::new (ptr) T();
    (void)typed;
  }
};

constexpr int kSingletonDefaultPriority = 0;

/// The lower the `Priority` the earlier singleton will be destroyed.
template <typename T, typename Tag = SingletonDefaultTag,
          int Priority = kSingletonDefaultPriority>
T& Singleton() {
  auto* res =
      singleton_internal::Singleton<T, Tag, Priority,
                                    SingletonTraits>::getInstance();
  return *res;
}

}  // namespace handbag
