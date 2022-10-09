#include "lib/cpp/io/input_memory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <string_view>

using namespace ::testing;

namespace handbag::io::tests {
namespace {
TEST(InMemoryInput, NonOwning) {
  const std::string_view haystack = "CONTENT";
  auto stream = makeNonOwningInMemoryInputStream(haystack);

  const auto all = readAll(stream);
  ASSERT_TRUE(all.ok());
  EXPECT_THAT(all.value(), Eq(haystack));
}
}  // namespace
}  // namespace handbag::io::tests
