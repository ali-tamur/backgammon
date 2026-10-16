# Backgammon AI

High-performance backgammon game engine in C++ with neural network training in Python.

## Prerequisites

- Bazelisk or Bazel 9.3 (for the C++ build; it downloads the LLVM 23.1.3 toolchain itself)
- Python 3.12
- Optional: an NVIDIA GPU for training (PyTorch's CUDA 12.6 build, which still supports Pascal cards; see `python/requirements.txt`)

### Install on Ubuntu/Debian

```bash
# Install Bazel (see https://bazel.build/install)

Setup

1. Clone and enter directory

git clone <repository-url>
cd backgammon

2. Generate Python protobuf files (uses the protoc Bazel builds; rerun after
   changing a .proto file)

python python/generate_protos.py

3. Set up Python environment

python -m venv .venv
source .venv/bin/activate  # On Windows: .venv\Scripts\activate
pip install -r python/requirements.txt

4. Build C++ binaries

# Build optimized binaries
bazel build -c opt \
  --copt='-march=native' \
  --copt='-flto' \
  --linkopt='-flto' \
  //cc/src:play_games_main

Usage

Generate Game Data

Play 100,000 games with random players and save to logs:

bazel run -c opt //cc/src:play_games_main -- \
  --w_model=random \
  --b_model=random \
  --n_games=100000 \
  --log_path=games/random_vs_random.pb \
  --take_turns

Convert Game Logs to Training Data

Every turn becomes one example: the position after the move from the mover's
side, labelled with how the game ended for the mover (see
cc/src/training_data.h). From Python, for matches stored in the database:

    from bg.training_data import training_data
    arrays = training_data(db, [match_id, ...])   # numpy arrays, cached in data/cache/

The converter also runs on its own:

bazel run -c opt //cc/src:training_data_main -- --games=/path/games.pb --out=/path/arrays/

Train Neural Network

# Activate Python environment
source .venv/bin/activate

# Training (PyTorch, exported to ONNX for the C++ engine) is being rewritten;
# see python/bg/.

Play Games with Trained Model

bazel run -c opt //cc/src:play_games_main -- \
  --w_model=my_model.onnx \
  --b_model=random \
  --n_games=1000 \
  --log_path=games/nn_vs_random.pb

Project Structure

.
├── cc/src/           # C++ game engine and tools
├── proto/            # Protocol buffer definitions
├── python/           # Python training code
├── tools/            # Developer scripts and git hooks
└── data/             # Database and caches (gitignored)

Development

Run C++ tests

bazel test //cc/src:all

Run benchmarks

bazel run -c opt //cc/src:game_manager_benchmark

Refresh compile_commands.json (for IDE)

python3 tools/compile_commands.py

Performance

- Game generation: ~4000 games/second (single-threaded)
- Training data conversion: ~100k games in 10 seconds



