#pragma once

#include "lib/cpp/io/internal/input_memory.h"

#include <utility>
#include <string_view>

namespace handbag::io {
template <typename T>
class OwningInMemoryInputStream final : public internal::InMemoryInputStream<T> {
using Base = internal::InMemoryInputStream<T>;
public:
    explicit OwningInMemoryInputStream(T data)
        : Base(std::move(data)) {}

    OwningInMemoryInputStream(const OwningInMemoryInputStream&) = delete;
    OwningInMemoryInputStream& operator=(const OwningInMemoryInputStream&) = delete;
    OwningInMemoryInputStream(OwningInMemoryInputStream&&) = default;
    OwningInMemoryInputStream& operator=(OwningInMemoryInputStream&&) = delete;

    using Base::read;
    using Base::close;
};

class NonOwningInMemoryInputStream final : public internal::InMemoryInputStream<std::string_view> {
using Base = internal::InMemoryInputStream<std::string_view>;
public:
    explicit NonOwningInMemoryInputStream(const std::string_view data) noexcept
        : Base(data) {}

    NonOwningInMemoryInputStream(const void* data, const size_t size) noexcept
        : NonOwningInMemoryInputStream(std::string_view(reinterpret_cast<const char*>(data), size)) {}

    NonOwningInMemoryInputStream(const NonOwningInMemoryInputStream&) = delete;
    NonOwningInMemoryInputStream& operator=(const NonOwningInMemoryInputStream&) = delete;
    NonOwningInMemoryInputStream(NonOwningInMemoryInputStream&&) = default;
    NonOwningInMemoryInputStream& operator=(NonOwningInMemoryInputStream&&) = delete;


    using Base::read;
    using Base::close;
};

inline NonOwningInMemoryInputStream makeNonOwningInMemoryInputStream(const std::string_view data) {
    return NonOwningInMemoryInputStream(data);
}

inline NonOwningInMemoryInputStream makeNonOwningInMemoryInputStream(const void* data, const size_t size) {
    return {data, size};
}

}
