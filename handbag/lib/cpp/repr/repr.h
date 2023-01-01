#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace handbag {

struct IRepr {
  virtual ~IRepr() = default;

  virtual std::string GetRepr() = 0;
};

class Repr {
  class NotPubliclyConstructible {};

 public:
  Repr(NotPubliclyConstructible /*npc*/, std::string_view name);
  Repr() = delete;
  Repr(const Repr&) = delete;
  Repr(Repr&&) = default;
  Repr& operator=(const Repr&) = delete;
  Repr& operator=(Repr&&) = default;

  static Repr create(std::string_view name) noexcept;

  template <typename T>
  Repr&& field(std::string_view name, const T& value) &&;

  std::string end() &&;

 private:
  bool has_fields_ = false;
  std::string repr_;
};

/// Impl

inline Repr::Repr(NotPubliclyConstructible /*npc*/, const std::string_view name)
    : repr_(name) {}

inline Repr Repr::create(const std::string_view name) noexcept {
  Repr res(NotPubliclyConstructible(), name);
  return res;
}

template <typename T>
inline Repr&& Repr::field(const std::string_view name, const T& value) && {
  std::ostringstream out(std::move(repr_));
  out << (has_fields_ ? ", " : "(") << name << "=" << value;
  has_fields_ = true;
  repr_ = std::move(out).str();
  return std::move(*this);
}

inline std::string Repr::end() && {
  auto res = std::move(repr_);
  return res;
}

}  // namespace handbag
