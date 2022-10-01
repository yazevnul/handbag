#pragma once

#include "lib/cpp/io/internal/input_memory.h"

#include <utility>
#include <string_view>

namespace handbag::io {
template <typename T>
class OwningInMemoryInputStream final : public internal::InMemoryInputStream<T> {
public:
    explicit OwningInMemoryInputStream(T data)
        : internal::InMemoryInputStream<T>(std::move(data)) {}
    OwningInMemoryInputStream(const OwningInMemoryInputStream&) = delete;
    OwningInMemoryInputStream& operator=(const OwningInMemoryInputStream&) = delete;
    OwnningInMemoryInputStream(OwningInMemoryInputStream&&) = default;
    OwningInMemoryInputStream& operator=(OwningInMemoryInputStream&&) = delete;

    using internal::InMemoryInputStream<T>::read;
    using internal::InMemoryInputStream<T>::close;
};

class NonOwningInMemoryInputStream final : public internal::InMemoryInputStream<std::string_view> {
public:
    explicit NonOwningInMemoryInputStream(const std::string_view data) noexcept
        : internal::InMemoryInputStream<std::string_view>(data) {}

    NonOwningInMemoryInputStream(const void* data, const size_t size) noexcept
        : NonOwningInMemoryInputStream({reinterpret_cast<const char*>(data), size}) {}

    NonOwningInMemoryInputStream(const NonOwningInMemoryInputStream&) = delete;
    NonOwningInMemoryInputStream& operator=(const NonOwningInMemoryInputStream&) = delete;
    NonOwnningInMemoryInputStream(NonOwningInMemoryInputStream&&) = default;
    NonOwningInMemoryInputStream& operator=(NonOwningInMemoryInputStream&&) = delete;


    using internal::InMemoryInputStream<T>::read;
    using internal::InMemoryInputStream<T>::close;
};

}
