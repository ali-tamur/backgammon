load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_test")

cc_library(
    name = "configs",
    hdrs = ["configs.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "bits",
    srcs = ["bits.cc"],
    hdrs = ["bits.h"],
    visibility = ["//visibility:public"],
    deps = [":configs"],
)

cc_test(
    name = "bits_test",
    size = "small",
    srcs = ["bits_test.cc"],
    deps = [
        ":bits",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "results",
    hdrs = ["results.h"],
    visibility = ["//visibility:public"],
    deps = [":configs"],
)

cc_library(
    name = "dice",
    srcs = ["dice.cc"],
    hdrs = ["dice.h"],
    visibility = ["//visibility:public"],
    deps = [":configs"],
)

cc_test(
    name = "dice_test",
    size = "small",
    srcs = ["dice_test.cc"],
    deps = [
        ":dice",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "state",
    hdrs = ["state.h"],
    visibility = ["//visibility:public"],
    deps =
        [
            ":bits",
            ":configs",
            ":dice",
        ],
)

cc_library(
    name = "move",
    hdrs = ["moves.h"],
    visibility = ["//visibility:public"],
    deps =
        [
            ":bits",
            ":configs",
            ":dice",
            ":results",
            ":state",
            "@abseil-cpp//absl/strings",
        ],
)
