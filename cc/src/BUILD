load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")
load("@rules_python//python:py_library.bzl", "py_library")
load("@rules_python//python:py_test.bzl", "py_test")
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_test")

cc_library(
    name = "backgammon_lib",
    srcs = [
        "dice.cc",
        "moves.cc",
        "zobrist_hash.cc",
    ],
    hdrs = [
        "bits.h",
        "configs.h",
        "dice.h",
        "moves.h",
        "py_interface.h",
        "results.h",
        "state.h",
        "state_hash.h",
        "zobrist_hash.h",
    ],
    deps = [
        "@abseil-cpp//absl/strings",
        # "@...//pybind11"
    ],
    alwayslink = 1,
)

pybind_extension(
    name = "backgammon",
    srcs = [
        "pybind_bridge.cc",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":backgammon_lib",
    ],
)

py_library(
    name = "backgammon_py",
    data = [":backgammon"],
    imports = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "py_interface",
    hdrs = ["py_interface.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "configs",
    hdrs = ["configs.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":py_interface",
    ],
)

cc_library(
    name = "bits",
    hdrs = ["bits.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":configs",
        ":py_interface",
    ],
)

cc_test(
    name = "bits_test",
    size = "small",
    srcs = ["bits_test.cc"],
    deps = [
        ":bits",
        ":configs",
        ":py_interface",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "dice",
    srcs = ["dice.cc"],
    hdrs = ["dice.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":configs",
        ":py_interface",
    ],
)

cc_test(
    name = "dice_test",
    size = "small",
    srcs = ["dice_test.cc"],
    deps = [
        ":dice",
        ":py_interface",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "results",
    hdrs = ["results.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":configs",
        ":py_interface",
        ":state_hash",
    ],
)

cc_library(
    name = "moves",
    srcs = ["moves.cc"],
    hdrs = ["moves.h"],
    visibility = ["//visibility:public"],
    deps =
        [
            ":bits",
            ":configs",
            ":dice",
            ":py_interface",
            ":results",
            ":state",
            "@abseil-cpp//absl/strings",
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
            ":py_interface",
            ":zobrist_hash",
        ],
)

cc_test(
    name = "state_test",
    size = "small",
    srcs = ["state_test.cc"],
    deps = [
        ":moves",
        ":py_interface",
        ":state",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "state_hash",
    hdrs = ["state_hash.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":py_interface",
        ":state",
    ],
)

cc_library(
    name = "zobrist_hash",
    srcs = ["zobrist_hash.cc"],
    hdrs = ["zobrist_hash.h"],
    visibility = ["//visibility:public"],
)
