#pragma once

#include <type_traits>
#include <memory>

#include "lib/cpp/singleton/internal/singleton.h"

namespace handbag {

/// @selfdocumenting
struct SingletonDefaultTag {};

template <typename T, typename Tag>
struct SingletonInstanceTraits {
  static void Construct(void* const ptr) noexcept(std::is_nothrow_default_constructible_v<T>) {
      ::new (ptr) T();
  }

  static void Destroy(void* const ptr) noexcept(std::is_nothrow_destructible_v<T>) {
      auto* const typed = reinterpret_cast<T*>(ptr);
      std::destroy_at(typed);
  }
};

template <typename T, typename Tag = SingletonDefaultTag, int Priority = 0>
T* Singleton() {
    auto* res = singleton_internal::Singleton<T, Tag, Priority, SingletonInstanceTraits>::getInstance();
    return *res;
}

}
