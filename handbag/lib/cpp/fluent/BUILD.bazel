cc_library(
    name = "fluent",
    srcs = ["fluent.cpp"],
    hdrs = ["fluent.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//lib/cpp/macro",
        "//lib/cpp/fluent/internal",
    ]
)

cc_test(
    name = "fluent_test",
    srcs = ["fluent_test.cpp"],
    deps = [
        "//lib/cpp/fluent",
        "@com_google_googletest//:gtest_main",
    ],
)
