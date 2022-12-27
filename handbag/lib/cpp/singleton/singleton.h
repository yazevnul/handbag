#pragma once

#include <memory>
#include <type_traits>

#include "lib/cpp/singleton/internal/singleton.h"

namespace handbag {

/// @selfdocumenting
struct SingletonDefaultTag {};

template <typename T, typename Tag>
struct SingletonInstanceTraits {
  static void Construct(void* const ptr) noexcept(
      std::is_nothrow_default_constructible_v<T>) {
    ::new (ptr) T();
  }
};

template <typename T, typename Tag = SingletonDefaultTag, int Priority = 0>
T& Singleton() {
  auto* res =
      singleton_internal::Singleton<T, Tag, Priority,
                                    SingletonInstanceTraits>::getInstance();
  return *res;
}

}  // namespace handbag
