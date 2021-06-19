#pragma once

#include "lib/cpp/macro/util.h"

#include <type_traits>

namespace handbag::fluent::internal {

template <typename T>
constexpr bool should_return_by_value() noexcept {
    if (!std::is_copy_constructible_v<T>) {
        return false;
    }

    const bool is_memory = sizeof(T) >= 4 * sizeof(size_t) && !std::is_trivially_copyable_v<T>;
    return std::is_integral_v<T> || std::is_floating_point_v<T> || !is_memory;
}

template <typename T>
constexpr bool should_pass_by_value() noexcept {
    const bool is_memory = sizeof(T) >= 4 * sizeof(size_t) && !std::is_trivially_copyable_v<T>;
    return std::is_integral_v<T> || std::is_floating_point_v<T> || !is_memory;
}


template <typename T, bool = should_return_by_value<T>()>
struct FluentRvDispatcher {
    using type = const T&;
};

template <typename T>
struct FluentRvDispatcher<T, true> {
    using type = T;
};

template <typename T>
using FluentRv = typename FluentRvDispatcher<T>::type;


template <typename T, bool = should_pass_by_value<T>()>
struct FluentParamDispatcher {
    using type = const T&;
};

template <typename T>
struct FluentParamDispatcher<T, true> {
    using type = T;
};

template <typename T>
using FluentParam = typename FluentParamDispatcher<T>::type;


template <typename T>
constexpr bool is_noexcept_setter() noexcept {
    return std::is_reference_v<FluentParam<T>>
        ? std::is_nothrow_copy_assignable_v<T>
        : std::is_nothrow_move_assignable_v<T>;
}

}

#define _HANDBAG_FLUENT_MEMBER_IMPL_1(type_) \
    static_assert(false, "`HANDBAG_FLUENT_MEMBER` can only have 2 or 3 arguments");

#define _HANDBAG_FLUENT_MEMBER_IMPL_2(type_, name)                                                                                                       \
private:                                                                                                                                                 \
    type_ _handbag_fluent_member_##name##_;                                                                                                              \
                                                                                                                                                         \
    static_assert(                                                                                                                                       \
        !std::is_trivial_v<type_>,                                                                                                                       \
        "Please use HANDBAG_FLUENT_MEMBER with 3 arguements; `" HANDBAG_STRINGIZE(type_) "` is trivial and must be initialized");                        \
                                                                                                                                                         \
public:                                                                                                                                                  \
    auto& set_##name(::handbag::fluent::internal::FluentParam<type_> new_value) & noexcept(::handbag::fluent::internal::is_noexcept_setter<type_>()) {   \
        _handbag_fluent_member_##name##_ = std::move(new_value);                                                                                         \
        return *this;                                                                                                                                    \
    }                                                                                                                                                    \
                                                                                                                                                         \
    auto&& set_##name(::handbag::fluent::internal::FluentParam<type_> new_value) && noexcept(::handbag::fluent::internal::is_noexcept_setter<type_>()) { \
        _handbag_fluent_member_##name##_ = std::move(new_value);                                                                                         \
        return std::move(*this);                                                                                                                         \
    }                                                                                                                                                    \
                                                                                                                                                         \
    ::handbag::fluent::internal::FluentRv<type_> name() const {                                                                                          \
        return _handbag_fluent_member_##name##_;                                                                                                         \
    }                                                                                                                                                    \
                                                                                                                                                         \
    const auto* ptr_##name() const noexcept {                                                                                                            \
        return &_handbag_fluent_member_##name##_;                                                                                                        \
    }                                                                                                                                                    \
                                                                                                                                                         \
    auto* mut_##name() noexcept {                                                                                                                        \
        return &_handbag_fluent_member_##name##_;                                                                                                        \
    }                                                                                                                                                    \
                                                                                                                                                         \
    auto&& move_##name() noexcept {                                                                                                                      \
        return std::move(_handbag_fluent_member_##name##_);                                                                                              \
    }                                                                                                                                                    \

#define _HANDBAG_FLUENT_MEMBER_IMPL_3(type_, name, default_)                                                                                             \
private:                                                                                                                                                 \
    type_ _handbag_fluent_member_##name##_ = default_;                                                                                                   \
                                                                                                                                                         \
public:                                                                                                                                                  \
    auto& set_##name(::handbag::fluent::internal::FluentParam<type_> new_value) & noexcept(::handbag::fluent::internal::is_noexcept_setter<type_>()) {   \
        _handbag_fluent_member_##name##_ = std::move(new_value);                                                                                         \
        return *this;                                                                                                                                    \
    }                                                                                                                                                    \
                                                                                                                                                         \
    auto&& set_##name(::handbag::fluent::internal::FluentParam<type_> new_value) && noexcept(::handbag::fluent::internal::is_noexcept_setter<type_>()) { \
        _handbag_fluent_member_##name##_ = std::move(new_value);                                                                                         \
        return std::move(*this);                                                                                                                         \
    }                                                                                                                                                    \
                                                                                                                                                         \
    ::handbag::fluent::internal::FluentRv<type_> name() const {                                                                                          \
        return _handbag_fluent_member_##name##_;                                                                                                         \
    }                                                                                                                                                    \
                                                                                                                                                         \
    const auto* ptr_##name() const noexcept {                                                                                                            \
        return &_handbag_fluent_member_##name##_;                                                                                                        \
    }                                                                                                                                                    \
                                                                                                                                                         \
    auto* mut_##name() noexcept {                                                                                                                        \
        return &_handbag_fluent_member_##name##_;                                                                                                        \
    }                                                                                                                                                    \
                                                                                                                                                         \
    auto&& move_##name() noexcept {                                                                                                                      \
        return std::move(_handbag_fluent_member_##name##_);                                                                                              \
    }                                                                                                                                                    \
