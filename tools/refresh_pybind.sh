#!/bin/bash
#
# I am too lazy to handle directories in a fool proof manner;
# Always run every script from the project top directory.
# (where .gitignore, MODULE.bazel etc. reside)
#
# Rebuilds the C++ move generator and installs it as python/src/backgammon.so.
# Run after changing C++ code that the Python side uses.
# Extra arguments are passed to bazel, e.g. ./tools/refresh_pybind.sh -c dbg

set -euo pipefail

TARGET="//cc/src:backgammon"
DEST="python/src/backgammon.so"
BAZEL_ARGS=(-c opt "$@")

bazel build "${BAZEL_ARGS[@]}" "${TARGET}"

# bazel-bin points at whichever configuration was built last; ask bazel instead.
SO_PATH="$(bazel cquery "${BAZEL_ARGS[@]}" --output=files "${TARGET}" 2>/dev/null | grep '\.so$')"

install -m 0644 "${SO_PATH}" "${DEST}"

.venv/bin/python -c "from python.src import backgammon; print('${DEST}: import OK')"
