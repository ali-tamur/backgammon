#!/bin/bash
#
# Sets up a clone for work: checks the tools that must be installed by hand,
# generates the Python protobuf code, creates .venv, enables the git hooks and
# creates the database if there is none. Safe to rerun at any time; rerun it
# after changing a .proto file or pulling changes to pyproject.toml.
#
# Installs nothing system-wide: missing tools are reported, with where to get
# them. Never touches an existing database (python -m bg new-db starts over).

set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

step() { echo "--- $1"; }
missing() {
  echo "MISSING: $1" >&2
  exit 1
}

step "tools"
command -v bazel >/dev/null || missing "bazel or bazelisk (https://bazel.build/install/bazelisk)"
command -v uv >/dev/null || missing "uv (https://docs.astral.sh/uv/getting-started/installation/)"
command -v python3 >/dev/null || missing "python3 (only to run tools/generate_protos.py before .venv exists)"
# Bazelisk honours .bazelversion; a plain bazel binary is whatever version it is.
want=$(cat .bazelversion)
have=$(bazel --version | awk '{print $2}')
if [ "$have" != "$want" ]; then
  echo "warning: bazel $have, the repository expects $want (.bazelversion)" >&2
fi
echo "bazel $have, $(uv --version)"

step "protobuf code (python/proto/)"
# Before uv sync: the editable install packages python/proto. The script only
# needs the standard library.
python3 tools/generate_protos.py

step "python environment (.venv)"
uv sync

step "git hooks"
git config core.hooksPath tools/githooks
echo "core.hooksPath = tools/githooks"

step "database"
if [ -e data/backgammon.db ]; then
  echo "data/backgammon.db exists, left as it is"
else
  uv run python -m bg new-db
fi

step "GPU (optional: training also runs on the CPU, slowly)"
uv run python -c 'import torch
if torch.cuda.is_available():
    print("PyTorch sees", torch.cuda.get_device_name(0))
else:
    print("PyTorch sees no GPU")'

echo
echo "Setup done. Activate the environment with: source .venv/bin/activate"
