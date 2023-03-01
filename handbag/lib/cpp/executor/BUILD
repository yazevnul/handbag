cc_library(
    name = "executor",
    srcs = [
        "executor.cpp",
        "internal/executor.cpp",
    ],
    hdrs = [
        "executor.h",
        "internal/executor.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/functional:any_invocable",
    ]
)

cc_library(
    name = "cpu",
    srcs = [
        "cpu.cpp",
    ],
    hdrs = [
        "cpu.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//lib/cpp/repr:repr",
        "//lib/cpp/start_stop:start_stop",
        ":executor",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/functional:any_invocable",
        "@com_google_absl//absl/log:log",
        "@com_google_absl//absl/synchronization:synchronization",
    ]
)
