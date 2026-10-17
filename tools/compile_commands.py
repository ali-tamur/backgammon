"""Writes .bazel/compile_commands.json for clangd and clang-tidy from Bazel's
compile actions (cc/.clangd points clangd there).

Run from the project top directory:  python3 tools/compile_commands.py [target pattern]
"""

import json
import os
import re
import subprocess
import sys

pattern = sys.argv[1] if len(sys.argv) > 1 else "//cc/..."


def bazel(*args):
    return subprocess.run(["bazel", *args], check=True, capture_output=True, text=True).stdout


output_base = bazel("info", "output_base").strip()
output_path = bazel("info", "output_path").strip()
aquery = json.loads(bazel("aquery", "--output=jsonproto", f'mnemonic("CppCompile", deps({pattern}))'))

# Compile commands use paths relative to Bazel's execroot: external/ for other
# repositories, bazel-out/ for generated files. Made absolute, they resolve from
# here; our sources (cc/...) are the same relative path in both places. Only a
# path at the start of an argument, after "=" or after -B is one of these, not
# one in the middle (bazel-out/k8-opt-exec/bin/external/...).
ROOTS = {"external": f"{output_base}/external", "bazel-out": output_path}
RELATIVE = re.compile(r"(^|=|^-B)(external|bazel-out)/")


def absolute(arg: str) -> str:
    return RELATIVE.sub(lambda m: f"{m[1]}{ROOTS[m[2]]}/", arg)


# clangd needs a real compiler as argv[0] (Bazel runs a wrapper script); use the
# compiler Bazel downloaded, so clangd and the build always agree.
clang = f"{output_base}/external/toolchains_llvm++llvm+llvm_toolchain_llvm/bin/clang++"
entries = []
for action in aquery.get("actions", []):
    args = action["arguments"]
    if "-c" not in args:  # header-only checks (e.g. abseil's parse_headers) compile no source
        continue
    entries.append(
        {
            "directory": os.getcwd(),
            "file": absolute(args[args.index("-c") + 1]),
            "arguments": [clang, *map(absolute, args[1:])],
        }
    )

os.makedirs(".bazel", exist_ok=True)
with open(".bazel/compile_commands.json", "w") as f:
    json.dump(entries, f, indent=1)
print(f".bazel/compile_commands.json: {len(entries)} entries")
