cc_library(
    name = "fwd",
    srcs = ["fwd.cpp"],
    hdrs = ["fwd.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "input",
    srcs = ["input.cpp"],
    hdrs = ["input.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":fwd",
        "@com_google_absl//absl/status:status",
        "@com_google_absl//absl/status:statusor",
    ]
)

cc_library(
    name = "input_memory",
    srcs = ["input_memory.cpp"],
    hdrs = ["input_memory.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//lib/cpp/io/internal:input_memory",
        "@com_google_absl//absl/status:status",
        "@com_google_absl//absl/status:statusor",
    ]
)

cc_test(
    name = "input_memory_test",
    srcs = ["input_memory_test.cpp"],
    deps = [
        ":input_memory",
        "@com_google_googletest//:gtest_main",
    ],
)
