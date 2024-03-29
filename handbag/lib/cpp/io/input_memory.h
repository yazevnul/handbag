#pragma once

#include <string_view>
#include <utility>

#include "lib/cpp/io/internal/input_memory.h"

namespace handbag::io {
template <typename T>
class OwningInMemoryInputStream final
    : public internal::InMemoryInputStream<T> {
  using Base = internal::InMemoryInputStream<T>;

 public:
  explicit OwningInMemoryInputStream(T data) : Base(std::move(data)) {}

  OwningInMemoryInputStream(const OwningInMemoryInputStream&) = delete;
  OwningInMemoryInputStream& operator=(const OwningInMemoryInputStream&) =
      delete;
  OwningInMemoryInputStream(OwningInMemoryInputStream&&) = default;
  OwningInMemoryInputStream& operator=(OwningInMemoryInputStream&&) = delete;

  using Base::close;
  using Base::read;
};

class NonOwningInMemoryInputStream final
    : public internal::InMemoryInputStream<std::string_view> {
  using Base = internal::InMemoryInputStream<std::string_view>;

 public:
  explicit NonOwningInMemoryInputStream(const std::string_view data) noexcept
      : Base(data) {}

  NonOwningInMemoryInputStream(const void* data, const size_t size) noexcept
      : NonOwningInMemoryInputStream(
            std::string_view(reinterpret_cast<const char*>(data), size)) {}

  NonOwningInMemoryInputStream(const NonOwningInMemoryInputStream&) = delete;
  NonOwningInMemoryInputStream& operator=(const NonOwningInMemoryInputStream&) =
      delete;
  NonOwningInMemoryInputStream(NonOwningInMemoryInputStream&&) = default;
  NonOwningInMemoryInputStream& operator=(NonOwningInMemoryInputStream&&) =
      delete;

  using Base::close;
  using Base::read;
};

inline NonOwningInMemoryInputStream makeNonOwningInMemoryInputStream(
    const std::string_view data) {
  return NonOwningInMemoryInputStream(data);
}

inline NonOwningInMemoryInputStream makeNonOwningInMemoryInputStream(
    const void* data, const size_t size) {
  const std::string_view view(reinterpret_cast<const char*>(data), size);
  return makeNonOwningInMemoryInputStream(view);
}

template <typename T>
inline OwningInMemoryInputStream<T> makeOwningInMemoryInputStream(T data) {
  return OwningInMemoryInputStream<T>(std::move(data));
}

}  // namespace handbag::io
