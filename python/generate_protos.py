"""Generates the Python code for proto/*.proto into python/proto/ (*_pb2.py and
.pyi stubs), importable as `from proto import game_pb2`: the editable install
of the project puts python/ on the path.

Uses the protoc that Bazel builds for the C++ side, so Python and C++ always
agree on the schema. Run from anywhere inside the repository, after any change
to a .proto file:

    python python/generate_protos.py

The Python protobuf package must be at least as new as this protoc (see
pyproject.toml); the generated code refuses to load otherwise.
"""

import subprocess
import sys
from pathlib import Path

PROTOC_TARGET = "@protobuf//:protoc"


def repo_root() -> Path:
    out = subprocess.run(["git", "rev-parse", "--show-toplevel"], capture_output=True, text=True, check=True)
    return Path(out.stdout.strip())


def build_protoc(root: Path) -> Path:
    subprocess.run(["bazel", "build", PROTOC_TARGET], cwd=root, check=True)
    # Ask bazel for the path: bazel-bin only reflects the last configuration built.
    out = subprocess.run(
        ["bazel", "cquery", "--output=files", PROTOC_TARGET], cwd=root, capture_output=True, text=True, check=True
    )
    return root / out.stdout.split()[0]


def main() -> int:
    root = repo_root()
    protoc = build_protoc(root)
    protos = sorted(str(p.relative_to(root)) for p in (root / "proto").glob("*.proto"))
    subprocess.run([str(protoc), "--python_out=python", "--pyi_out=python", *protos], cwd=root, check=True)
    for proto in protos:
        print(f"generated python/{proto.removesuffix('.proto')}_pb2.py and .pyi")
    return 0


if __name__ == "__main__":
    sys.exit(main())
