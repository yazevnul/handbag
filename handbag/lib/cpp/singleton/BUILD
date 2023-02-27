cc_library(
    name = "singleton",
    srcs = [
        "fwd.cpp",
        "internal/singleton.cpp",
        "singleton.cpp",
    ],
    hdrs = [
        "fwd.h",
        "internal/singleton.h",
        "singleton.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/cleanup:cleanup",
        "@com_google_absl//absl/memory:memory",
    ]
)

