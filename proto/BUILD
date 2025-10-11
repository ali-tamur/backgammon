
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "game_proto",
    srcs = ["game.proto"],
    visibility = ["//visibility:public"],
)

cc_proto_library(
    name = "game_cc_proto",
    deps = [":game_proto"],
    visibility = ["//visibility:public"],
)
