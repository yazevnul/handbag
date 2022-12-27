cc_library(
    name = "singleton",
    srcs = [
        "singleton.cpp",
        "internal/singleton.cpp"
    ],
    hdrs = [
        "singleton.h",
        "internal/singleton.h"
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory:memory",
        "@com_google_absl//absl/cleanup:cleanup",
    ]
)

