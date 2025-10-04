
# Load the standard Python rules from @rules_python
load("@rules_python//python:defs.bzl", "py_binary")

# Define the Python executable target.
# This target runs your test_bridge.py script.
py_binary(
    name = "test_bridge",
    srcs = ["test_bridge.py"],
    
    deps = [
        "//cc/src:backgammon",
    ],
)

