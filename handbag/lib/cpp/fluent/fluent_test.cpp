#include "lib/cpp/fluent/fluent.h"

#include <memory>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;

namespace {
struct Foo {
  HANDBAG_FLUENT_MEMBER(int, number, 7)
  HANDBAG_FLUENT_MEMBER(std::string, str)
  HANDBAG_FLUENT_MEMBER(std::string, str_with_default, "default")
};

TEST(FluentTest, Ctor) {
  const Foo foo;
  EXPECT_THAT(foo, Property(&Foo::number, Eq(7)));
  EXPECT_THAT(foo, Property(&Foo::str, IsEmpty()));
  EXPECT_THAT(foo, Property(&Foo::str_with_default, Eq("default")));
}

struct Bar {
  HANDBAG_FLUENT_MEMBER(int, one, 1)
  HANDBAG_FLUENT_MEMBER(int, two, 2)
  HANDBAG_FLUENT_MEMBER(std::unique_ptr<int>, move_only)
};

TEST(FluentTest, MoveOnly) {
  {
    const Bar bar;
    EXPECT_THAT(bar, Property(&Bar::move_only, IsNull()));
  }

  {
    const auto bar =
        Bar().set_move_only(std::make_unique<int>(3)).set_one(11).set_two(22);
    EXPECT_THAT(bar, Property(&Bar::one, Eq(11)));
    EXPECT_THAT(bar, Property(&Bar::two, Eq(22)));
    EXPECT_THAT(bar, Property(&Bar::move_only, Pointee(Eq(3))));
  }
}

TEST(FluentTest, Ptrs) {
  auto foo =
      Foo().set_number(1).set_str("str").set_str_with_default("not_default");

  EXPECT_THAT(foo, Property(&Foo::ptr_number, NotNull()));
  EXPECT_THAT(foo.mut_number(), NotNull());
}

TEST(FluentTest, Move) {
  auto foo =
      Foo().set_number(1).set_str("str").set_str_with_default("not_default");

  const auto sink = std::move(foo.move_str());
  EXPECT_EQ(sink, "str");
}
}  // namespace
