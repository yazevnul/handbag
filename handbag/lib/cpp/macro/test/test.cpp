#include "gtest/gtest.h"

#include "lib/cpp/macro/util.h"
#include "lib/cpp/macro/va_args.h"

TEST(MacroTest, Stringize) {
    EXPECT_STREQ(HANDBAG_STRINGIZE(test), "test");
    EXPECT_STREQ(HANDBAG_STRINGIZE(HANDBAG_STRINGIZE(test)), "\"test\"");
}

#define HANDBAG_TEST_MACRO_1(X) HANDBAG_STRINGIZE(X)
#define HANDBAG_TEST_MACRO_2(X, Y) HANDBAG_STRINGIZE(Y)
#define HANDBAG_TEST_MACRO(...) HANDBAG_PASS_VA_ARGS(HANDBAG_MACRO_OVERLOAD_DISPATCHER_2(__VA_ARGS__, HANDBAG_TEST_MACRO_2, HANDBAG_TEST_MACRO_1)(__VA_ARGS__))

TEST(MacroTest, MacroOverloadDispatcher) {
    EXPECT_STREQ(HANDBAG_TEST_MACRO(X), "X");
    EXPECT_STREQ(HANDBAG_TEST_MACRO(X, Y), "Y");
}
