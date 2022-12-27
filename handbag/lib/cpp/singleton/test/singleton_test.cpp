#include "lib/cpp/singleton/singleton.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <future>
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

TEST(SingletonTest, SamePtr) {
  auto* const first = &Singleton<Foo>();
  auto* const second = &Singleton<Foo>();
  EXPECT_THAT(first, Pointer(Eq(second)));
}

TEST(SingletonTest, PodDefaultInit) {
  const auto value = Singleton<int>();
  EXPECT_THAT(value, Eq(0));
}

TEST(SingletonTest, CustomTraits) {
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

TEST(SingletonTest, Retry) {
  EXPECT_THAT(
      [] {
        const auto& foo = Singleton<FirstCallToCtorThrows>();
        (void)foo;
      },
      ThrowsMessage<std::runtime_error>(Eq(FirstCallToCtorThrows::NEEDLE)));
  EXPECT_NO_THROW({
    const auto& foo = Singleton<FirstCallToCtorThrows>();
    (void)foo;
  });
}

struct CountsCtorCalls {
  static std::atomic<int> ctor_calls;

  CountsCtorCalls() { (void)ctor_calls.fetch_add(1); }
};

std::atomic<int> CountsCtorCalls::ctor_calls = 0;

TEST(SingletonTest, CtorCalls) {
  EXPECT_THAT(CountsCtorCalls::ctor_calls.load(std::memory_order_acquire),
              Eq(0));
  const auto& foo = Singleton<CountsCtorCalls>();
  (void)foo;
  EXPECT_THAT(CountsCtorCalls::ctor_calls.load(std::memory_order_acquire),
              Eq(1));
  const auto& bar = Singleton<CountsCtorCalls>();
  (void)bar;
  EXPECT_THAT(CountsCtorCalls::ctor_calls.load(std::memory_order_acquire),
              Eq(1));
}

struct ToBeAllocatedOnHeap {
  static constexpr size_t kSize = 10ULL * 1024 * 1024;

  std::array<std::byte, kSize> data;
};

TEST(SingletonTest, OnHeap) { (void)Singleton<ToBeAllocatedOnHeap>(); }

TEST(SingletonTest, Concurrency) {
  struct A {};
  const auto task = [] {
    const auto& foo = Singleton<A>();
    (void)foo;
  };
  auto job_one = std::async(std::launch::async, task);
  auto job_two = std::async(std::launch::async, task);

  job_one.get();
  job_two.get();
}

struct ForDifferentPriorities {};

TEST(SingletonDeathTest, DeathOnDifferentPriorities) {
  const auto& foo = Singleton<ForDifferentPriorities>();
  (void)foo;
  EXPECT_DEATH(([] {
                 const auto& bar =
                     Singleton<ForDifferentPriorities, SingletonDefaultTag,
                               kSingletonDefaultPriority + 1>();
                 (void)bar;
               }()),
               "priority");
}

}  // namespace
}  // namespace handbag::tests
