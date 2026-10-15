#!/bin/bash
#
# I am too lazy to handle directories in a fool proof manner;
# Always run every script from the project top directory.
# (where .gitignore, MODULE.bazel etc. reside)

if [ -n "$VIRTUAL_ENV" ]; then
      echo "Error: Deactivate your Python virtual environment first!"
      exit 1
fi

if [ $# -lt 2 ]; then
      echo "Usage: ./convert_to_onnx.sh <input.keras> <output.onnx>"
      exit 1
fi

MODEL_NAME="$1"
OUTPUT_NAME="$2"

KERAS_DIR="$(pwd)/models/keras/${MODEL_NAME}"
OUTPUT_DIR="$(pwd)/models/onnx"

mkdir -p "$OUTPUT_DIR"

docker build -t keras-to-onnx tools/

docker run --rm                          \
  -v "${KERAS_DIR}:/workspace/model"     \
  -v "${OUTPUT_DIR}:/workspace/output"   \
  -v "$(pwd)/tools:/tools"               \
  keras-to-onnx /tools/keras_to_onnx.py  \
  /workspace/model                       \
  /workspace/output/${OUTPUT_NAME}

