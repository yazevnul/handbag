#include "lib/cpp/io/input_memory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <string_view>

using namespace ::testing;

namespace handbag::io::tests {
namespace {
TEST(InMemoryInput, ReadAllNonOwning) {
  const std::string_view expected = "CONTENT";
  auto stream = makeNonOwningInMemoryInputStream(expected);

  const auto all = readAll(stream);
  ASSERT_TRUE(all.ok());
  EXPECT_THAT(all.value(), Eq(expected));
}

TEST(InMemoryInput, ReadAllNonOwning2) {
  const std::string_view expected = "CONTENT";
  auto stream =
      makeNonOwningInMemoryInputStream(expected.data(), expected.size());

  const auto all = readAll(stream);
  ASSERT_TRUE(all.ok());
  EXPECT_THAT(all.value(), Eq(expected));
}

TEST(InMemoryInput, ReadAllOwningString) {
  const std::string expected = "CONTENT";
  auto stream = makeOwningInMemoryInputStream(expected);

  const auto all = readAll(stream);
  ASSERT_TRUE(all.ok());
  EXPECT_THAT(all.value(), Eq(expected));
}

TEST(InMemoryInput, ReadAllOwningVector) {
  const std::string expected = "CONTENT";
  auto stream = makeOwningInMemoryInputStream(expected);

  const auto all = readAll(stream);
  ASSERT_TRUE(all.ok());
  EXPECT_THAT(all.value(), Eq(expected));
}
}  // namespace
}  // namespace handbag::io::tests
