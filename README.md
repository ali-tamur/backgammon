# Backgammon AI

High-performance backgammon game engine in C++ with neural network training in Python.

## Prerequisites

- Bazelisk or Bazel 9.3 (for the C++ build; it downloads the LLVM 23.1.3 toolchain itself)
- [uv](https://docs.astral.sh/uv/) (it installs Python 3.14 itself)
- `python3`, `git`, and `zstd` (to unpack an archived database)
- Optional: an NVIDIA GPU for training (PyTorch's CUDA 12.6 build, which still supports Pascal cards; see `pyproject.toml`)

## Setup

```bash
git clone <repository-url>
cd backgammon
tools/setup.sh
source .venv/bin/activate  # bg and proto are importable from anywhere
```

`tools/setup.sh` checks the tools above, generates the Python protobuf code
(with the protoc Bazel builds), creates `.venv` exactly as `uv.lock` says,
enables the git hooks and creates `data/backgammon.db` if there is none. It
installs nothing system-wide and is safe to rerun: do so after changing a
`.proto` file or pulling changes to `pyproject.toml`.

To change dependencies, edit `pyproject.toml`, then run `uv lock` and
`uv sync`, and commit `uv.lock` with the change.

## Usage

Operations are commands; `python -m bg --help` lists them.

```bash
python -m bg new-db  # start over: archives data/backgammon.db to data/archive/
```

Exploring the data is done in Python, with the models in `bg.db`:

```python
from bg.db import Match, open_db
from bg.training_data import training_data

with open_db("data/backgammon.db"):
    arrays = training_data([Match.get_by_id(i) for i in match_ids])  # cached in data/cache/
```

Every turn becomes one training example: the position after the move from the
mover's side, labelled with how the game ended for the mover (see
`cc/src/training_data.h`).

Training (PyTorch, exported to ONNX for the C++ engine) is being rewritten;
see `python/bg/`.

### The C++ binaries on their own

```bash
# Optimized build
bazel build -c opt --copt='-march=native' --copt='-flto' --linkopt='-flto' //cc/src:play_games_main

# 100,000 games between random players (refuses an existing log file)
bazel run -c opt //cc/src:play_games_main -- \
  --w_model=random --b_model=random --n_games=100000 \
  --log_path=/tmp/random_vs_random.pb --take_turns

# A network against a random player
bazel run -c opt //cc/src:play_games_main -- \
  --w_model=my_model.onnx --b_model=random --n_games=1000 \
  --log_path=/tmp/nn_vs_random.pb

# Game log to training arrays
bazel run -c opt //cc/src:training_data_main -- --games=/path/games.pb --out=/path/arrays/
```

## Project structure

```
.
├── cc/src/   # C++ game engine and tools
├── proto/    # Protocol buffer definitions
├── python/   # Python: bg/ (ours), proto/ (generated), tests/
├── tools/    # Setup, developer scripts and git hooks
└── data/     # Database, its archives and caches (gitignored)
```

## Development

```bash
bazel test //...                                   # C++ tests
pytest                                             # Python tests
bazel run -c opt //cc/src:game_manager_benchmark   # benchmarks
python3 tools/compile_commands.py                  # refresh compile_commands.json (for the IDE)
```

The pre-commit hook (`tools/githooks/pre-commit`) runs ruff, pytest,
clang-format, clang-tidy and `bazel test //...`.

## Performance

- Game generation: ~4000 games/second (single-threaded)
- Training data conversion: ~100k games in 10 seconds
