cc_library(
      name = "onnxruntime",
      srcs = [
          "onnxruntime-linux-x64-1.20.1/lib/libonnxruntime.so.1.20.1",
          "onnxruntime-linux-x64-1.20.1/lib/libonnxruntime.so.1",
          "onnxruntime-linux-x64-1.20.1/lib/libonnxruntime.so",
      ],
      hdrs = glob(["onnxruntime-linux-x64-1.20.1/include/**/*.h"]),
      includes = ["onnxruntime-linux-x64-1.20.1/include"],
      data = glob(["onnxruntime-linux-x64-1.20.1/lib/libonnxruntime.so*"]),
      visibility = ["//visibility:public"],
  )
