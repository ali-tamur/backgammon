#!/bin/bash
#
# I am too lazy to handle directories in a fool proof manner;
# Always run every script from the project top directory.
# (where .gitignore, MODULE.bazel etc. reside)
#
find cc/. -type f \( -name "*.cc" -o -name "*.h" \) | xargs clang-format -i --style=file

