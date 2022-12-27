#include "lib/cpp/singleton/singleton.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <string_view>

using namespace ::testing;

namespace handbag {
namespace {
class IntTagOne;
constexpr int kIntTagOneValue = 20221227;
}  // namespace

template <>
struct SingletonInstanceTraits<int, IntTagOne> {
  static void Construct(void* const ptr) noexcept {
    ::new (ptr) int(kIntTagOneValue);
  }
};
}  // namespace handbag

namespace handbag::tests {
namespace {

struct Foo {};

TEST(Singleton, Test) {
  auto* const first = &Singleton<Foo>();
  auto* const second = &Singleton<Foo>();
  EXPECT_THAT(first, Pointer(Eq(second)));
}

TEST(Singleton, POD) {
  const auto value = Singleton<int>();
  EXPECT_THAT(value, Eq(0));
}

TEST(Singleton, CustomTraits) {
  const auto value = Singleton<int, IntTagOne>();
  EXPECT_THAT(value, Eq(kIntTagOneValue));
}

struct FirstCallToCtorThrows {
  static constexpr std::string_view NEEDLE = "NEEDLE";

  FirstCallToCtorThrows() {
    static std::atomic<int> attempts = 0;
    const auto attempt = attempts.fetch_add(1);
    if (attempt == 0) {
      throw std::runtime_error(std::string(NEEDLE));
    }
  }
};

TEST(Singleton, Retry) {
  EXPECT_THAT(
      [] { (void)Singleton<FirstCallToCtorThrows>(); },
      ThrowsMessage<std::runtime_error>(Eq(FirstCallToCtorThrows::NEEDLE)));
  EXPECT_NO_THROW({ (void)Singleton<FirstCallToCtorThrows>(); });
}

struct CountsCtorCalls {
  static std::atomic<int> ctor_calls;

  CountsCtorCalls() { (void)ctor_calls.fetch_add(1); }
};

std::atomic<int> CountsCtorCalls::ctor_calls = 0;

TEST(Singleton, CtorCalls) {
  EXPECT_THAT(CountsCtorCalls::ctor_calls.load(std::memory_order_acquire),
              Eq(0));
  (void)Singleton<CountsCtorCalls>();
  EXPECT_THAT(CountsCtorCalls::ctor_calls.load(std::memory_order_acquire),
              Eq(1));
  (void)Singleton<CountsCtorCalls>();
  EXPECT_THAT(CountsCtorCalls::ctor_calls.load(std::memory_order_acquire),
              Eq(1));
}

}  // namespace
}  // namespace handbag::tests
