import subprocess
import sys


def convert_to_onnx(saved_model_path, onnx_path):
    """Convert TensorFlow SavedModel to ONNX using tf2onnx CLI."""
    print(f"Converting {saved_model_path} to ONNX...")

    result = subprocess.run(
        ["python", "-m", "tf2onnx.convert", "--saved-model", saved_model_path, "--output", onnx_path, "--opset", "13"],
        capture_output=True,
        text=True,
    )

    print(result.stdout)
    if result.returncode != 0:
        print(result.stderr)
        sys.exit(1)

    print(f"Successfully converted to: {onnx_path}")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python keras_to_onnx.py <saved_model_dir> <output.onnx>")
        sys.exit(1)

    convert_to_onnx(sys.argv[1], sys.argv[2])
