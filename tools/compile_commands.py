"""Writes compile_commands.json for clangd from Bazel's compile actions.

Run from the project top directory:  python3 tools/compile_commands.py [target pattern]
"""

import json
import os
import subprocess
import sys

pattern = sys.argv[1] if len(sys.argv) > 1 else "//cc/..."


def bazel(*args):
    return subprocess.run(["bazel", *args], check=True, capture_output=True, text=True).stdout


output_base = bazel("info", "output_base").strip()
aquery = json.loads(bazel("aquery", "--output=jsonproto", f'mnemonic("CppCompile", deps({pattern}))'))

# Compile commands are relative to the execroot; these symlinks make them resolve from here.
for name, target in [("external", f"{output_base}/external"), ("bazel-out", bazel("info", "output_path").strip())]:
    if os.path.islink(name) and os.readlink(name) != target:
        os.remove(name)  # stale: left over from another output base (e.g. after moving the project)
    if not os.path.lexists(name):
        os.symlink(target, name)

# clangd needs a real compiler as argv[0] (Bazel runs a wrapper script); use the
# compiler Bazel downloaded, so clangd and the build always agree.
clang = os.path.abspath("external/toolchains_llvm++llvm+llvm_toolchain_llvm/bin/clang++")
entries = []
for action in aquery.get("actions", []):
    args = action["arguments"]
    if "-c" not in args:  # header-only checks (e.g. abseil's parse_headers) compile no source
        continue
    entries.append({"directory": os.getcwd(), "file": args[args.index("-c") + 1], "arguments": [clang, *args[1:]]})

with open("compile_commands.json", "w") as f:
    json.dump(entries, f, indent=1)
print(f"compile_commands.json: {len(entries)} entries")
