# BUILD file for Microsoft's prebuilt onnxruntime release archive (see MODULE.bazel).
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "onnxruntime",
    srcs = [
        "lib/libonnxruntime.so",
        "lib/libonnxruntime.so.1",
        "lib/libonnxruntime.so.1.20.1",
    ],
    hdrs = glob(["include/**/*.h"]),
    data = glob(["lib/libonnxruntime.so*"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
