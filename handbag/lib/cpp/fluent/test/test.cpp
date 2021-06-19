#include "gtest/gtest.h"

#include "lib/cpp/fluent/fluent.h"

#include <string>
#include <memory>

namespace {
    struct Foo {
        HANDBAG_FLUENT_MEMBER(int, number, 7)
        HANDBAG_FLUENT_MEMBER(std::string, str)
        HANDBAG_FLUENT_MEMBER(std::string, str_with_default, "default")
    };
}

TEST(FluentTest, Ctor) {
    const Foo foo;
    EXPECT_EQ(7, foo.number());
    EXPECT_EQ("", foo.str());
    EXPECT_EQ("default", foo.str_with_default());
}

namespace {
    struct Bar {
        HANDBAG_FLUENT_MEMBER(int, one, 1)
        HANDBAG_FLUENT_MEMBER(int, two, 2)
        HANDBAG_FLUENT_MEMBER(std::unique_ptr<int>, move_only)
    };
}

TEST(FluentTest, MoveOnly) {
    {
        const Bar bar;
        EXPECT_EQ(nullptr, bar.move_only());
    }
    {
        const auto bar = Bar()
            .set_move_only(std::make_unique<int>(3))
            .set_one(11)
            .set_two(22);
        EXPECT_EQ(11, bar.one());
        EXPECT_EQ(22, bar.two());
        EXPECT_EQ(3, *bar.move_only());
    }
}

TEST(FluentTest, Ptrs) {
    auto foo = Foo()
        .set_number(1)
        .set_str("str")
        .set_str_with_default("not_default");

    EXPECT_NE(nullptr, foo.ptr_number());
    EXPECT_NE(nullptr, foo.mut_number());
}

TEST(FluentTest, Move) {
    auto foo = Foo()
        .set_number(1)
        .set_str("str")
        .set_str_with_default("not_default");

    const auto sink = std::move(foo.move_str());
    EXPECT_EQ("str", sink);
    EXPECT_EQ("", foo.str());
}
