# Backgammon AI

High-performance backgammon game engine in C++ with neural network training in Python.

## Prerequisites

- Bazelisk or Bazel 9.3 (for the C++ build; it downloads the LLVM 23.1.3 toolchain itself)
- Python 3.12 (the C++ module is built for the version set in `MODULE.bazel`)
- Docker (for model conversion)
- Protocol Buffers compiler (`protoc`)

### Install on Ubuntu/Debian

```bash
# Install protoc
sudo apt install protobuf-compiler

# Install Bazel (see https://bazel.build/install)
# Install Docker (see https://docs.docker.com/engine/install/)

Setup

1. Clone and enter directory

git clone <repository-url>
cd backgammon

2. Generate Python protobuf files

protoc --python_out=. proto/game.proto

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
  //cc/src:play_games_main \
  //cc/src:training_data_main

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

bazel run -c opt //cc/src:training_data_main -- \
  --log_path=/games/random_vs_random.pb \
  --output_path=/training/train_data.pb

Train Neural Network

# Activate Python environment
source .venv/bin/activate

# Run training notebook or script
jupyter lab python/notebooks/train.ipynb

Convert Keras Model to ONNX

# Build Docker image (first time only)
docker build -t keras-to-onnx tools/

# Convert model
./tools/convert_to_onnx.sh \
  models/my_model.keras \
  models/my_model.onnx

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
├── tools/            # Utility scripts (model conversion)
├── games/            # Game log storage (gitignored)
├── models/           # Trained models (gitignored)
└── training/         # Training data (gitignored)

Development

Run C++ tests

bazel test //cc/src:all

Run benchmarks

bazel run -c opt //cc/src:game_manager_benchmark

Refresh compile_commands.json (for IDE)

bazel run //cc:refresh_cc_for_backgammon

Performance

- Game generation: ~4000 games/second (single-threaded)
- Training data conversion: ~100k games in 10 seconds
- Training: ~4.7M examples in ~10 minutes (GPU)



