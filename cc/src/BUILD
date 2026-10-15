load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("@rules_python//python:py_library.bzl", "py_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "backgammon_lib",
    srcs = [
        "dice.cc",
        "moves.cc",
        "zobrist_hash.cc",
    ],
    hdrs = [
        "assertion.h",
        "bits.h",
        "configs.h",
        "dice.h",
        "moves.h",
        "results.h",
        "state.h",
        "state_hash.h",
        "zobrist_hash.h",
    ],
    deps = [
        # "@...//pybind11"
    ],
    alwayslink = 1,
)

pybind_extension(
    name = "backgammon",
    srcs = [
        "pybind_bridge.cc",
    ],
    deps = [
        ":backgammon_lib",
    ],
)

py_library(
    name = "backgammon_py",
    data = [":backgammon"],
    imports = ["."],
)

cc_library(
    name = "configs",
    hdrs = ["configs.h"],
)

cc_library(
    name = "assertion",
    hdrs = ["assertion.h"],
)

cc_library(
    name = "bits",
    hdrs = ["bits.h"],
    deps = [
        ":configs",
    ],
)

cc_library(
    name = "pretty_print",
    hdrs = ["pretty_print.h"],
    srcs = ["pretty_print.cc"],
    deps = [
        ":assertion",
        ":configs",
    ],
)

cc_test(
    name = "pretty_print_test",
    srcs = ["pretty_print_test.cc"],
    deps = [
        ":assertion",
        ":configs",
        ":pretty_print",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "bits_test",
    size = "small",
    srcs = ["bits_test.cc"],
    deps = [
        ":bits",
        ":configs",
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
    ],
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
    name = "model",
    hdrs = ["model.h"],
    data = ["//third_party:onnxruntime"],
    srcs = ["model.cc"],
    deps = [
        ":configs",
        "//proto:game_cc_proto",
        "//third_party:onnxruntime",
    ],
)

cc_library(
    name = "player",
    hdrs = ["player.h"],
    deps = [
        ":dice",
        ":model",
        ":moves",
        "//third_party:onnxruntime",
    ],
)

cc_library(
    name = "training_data",
    srcs = ["training_data.cc"],
    hdrs = ["training_data.h"],
    deps = [
        ":assertion",
        ":configs",
        ":logger",
        ":model",
        "//proto:game_cc_proto",
    ],
)

cc_library(
    name = "logger",
    srcs = ["logger.cc"],
    hdrs = ["logger.h"],
    deps = [
        ":assertion",
        ":configs",
        ":dice",
        "//proto:game_cc_proto",
    ],
)

cc_library(
    name = "results",
    hdrs = ["results.h"],
    deps = [
        ":assertion",
        ":configs",
        ":state",
        ":state_hash",
    ],
)

cc_library(
    name = "moves",
    srcs = ["moves.cc"],
    hdrs = ["moves.h"],
    deps =
        [
        ":assertion",
            ":bits",
            ":configs",
            ":dice",
            ":results",
            ":state",
        ],
)

cc_library(
    name = "state",
    hdrs = ["state.h"],
    deps =
        [
            ":assertion",
            ":bits",
            ":configs",
            ":dice",
            ":zobrist_hash",
        ],
)

cc_test(
    name = "state_test",
    size = "small",
    srcs = ["state_test.cc"],
    deps = [
        ":moves",
        ":state",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "state_hash",
    hdrs = ["state_hash.h"],
    deps = [
        ":state",
    ],
)

cc_library(
    name = "zobrist_hash",
    srcs = ["zobrist_hash.cc"],
    hdrs = ["zobrist_hash.h"],
)

cc_library(
    name = "game_manager",
    srcs = ["game_manager.cc"],
    hdrs = ["game_manager.h"],
    deps = [
        ":assertion",
        ":configs",
        ":dice",
        ":logger",
        ":pretty_print",
        ":model",
        ":moves",
        ":player",
        "//third_party:onnxruntime",
    ],
)

cc_binary(
    name = "game_manager_benchmark",
    srcs = ["game_manager_benchmark.cc"],
    copts = ["-pg"],
    linkopts = ["-pg"],
    deps = [
        ":configs",
        ":dice",
        ":game_manager",
        ":logger",
        ":model",
        ":moves",
        ":player",
        "//third_party:onnxruntime",
        "@google_benchmark//:benchmark",
    ],
)

cc_test(
    name = "game_manager_test",
    size = "small",
    srcs = ["game_manager_test.cc"],
    data = ["//third_party:onnxruntime"],
    deps = [
        ":configs",
        ":dice",
        ":game_manager",
        ":logger",
        ":model",
        ":moves",
        ":player",
        "//third_party:onnxruntime",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "play_games_main",
    srcs = ["play_games_main.cc"],
    copts = ["-pg"],
    linkopts = ["-pg"],
    deps = [
        ":assertion",
        ":configs",
        ":dice",
        ":game_manager",
        ":logger",
        ":model",
        ":moves",
        ":player",
        "//third_party:onnxruntime",
        "@cli11//:cli11",
    ],
)

cc_binary(
    name = "training_data_main",
    srcs = ["training_data_main.cc"],
    copts = ["-pg"],
    linkopts = ["-pg"],
    deps = [
        ":assertion",
        ":configs",
        ":training_data",
        "//proto:game_cc_proto",
        "@cli11//:cli11",
    ],
)