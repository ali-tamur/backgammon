load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@protobuf//bazel:proto_library.bzl", "proto_library")

proto_library(
    name = "game_proto",
    srcs = ["game.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "game_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":game_proto"],
)
