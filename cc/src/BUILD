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
    name = "model",
    hdrs = ["model.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//third_party:onnxruntime",
    ],
)


cc_library(
    name = "player",
    hdrs = ["player.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//third_party:onnxruntime",
        ":dice", 
        ":moves",
        ":py_interface",
        ":model"
    ],
)

cc_library(
    name = "logger",
    hdrs = ["logger.h"],
    srcs = ["logger.cc"],
    visibility = ["//visibility:public"],
    deps = [
        ":py_interface",
        ":configs",
        ":dice",
        "//proto:game_cc_proto"
    ],
)

cc_library(
    name = "game_manager",
    hdrs = ["game_manager.h"],
    srcs = ["game_manager.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//third_party:onnxruntime",
        ":configs",
        ":dice", 
        ":logger",
        ":moves",
        ":py_interface",
        ":model",
        ":player"
    ],
)

cc_binary(
      name = "game_manager_benchmark",
      srcs = ["game_manager_benchmark.cc"],
      deps = [
        "//third_party:onnxruntime",
        ":game_manager",
        ":configs",
        ":dice", 
        ":logger",
        ":moves",
        ":py_interface",
        ":model",
        ":player",
          "@google_benchmark//:benchmark",
      ],
      linkopts = ["-pg"],
      copts = ["-pg"],
  )

cc_test(
    name = "game_manager_test",
    size = "small",
    srcs = ["game_manager_test.cc"],
    data = ["//third_party:onnxruntime"],
    deps = [
        ":game_manager",
        ":configs",
        ":dice", 
        ":moves",
        ":logger",
        ":py_interface",
        ":model",
        ":player",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
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
