"""Writes test_model.onnx, a tiny network whose outputs tests can compute by hand.

    probabilities = softmax(W @ x + b),  W[j][i] = WEIGHT(i, j),  b[j] = BIAS(j)

cc/src/model_test.cc repeats the same formulas. Rerun after changing them:

    python cc/src/testdata/make_test_model.py
"""

from pathlib import Path

import torch

INPUTS = 248
OUTPUTS = 4


def weight(i: int, j: int) -> float:
    return ((i * 7 + j * 13) % 17 - 8) / 8


def bias(j: int) -> float:
    return (j - 1.5) / 4


def main() -> None:
    linear = torch.nn.Linear(INPUTS, OUTPUTS)
    with torch.no_grad():
        linear.weight.copy_(torch.tensor([[weight(i, j) for i in range(INPUTS)] for j in range(OUTPUTS)]))
        linear.bias.copy_(torch.tensor([bias(j) for j in range(OUTPUTS)]))
    model = torch.nn.Sequential(linear, torch.nn.Softmax(dim=1)).eval()
    path = Path(__file__).with_name("test_model.onnx")
    torch.onnx.export(
        model,
        (torch.zeros(1, INPUTS),),
        path,
        input_names=["board"],
        output_names=["probabilities"],
        dynamic_shapes=({0: torch.export.Dim("batch")},),
        dynamo=True,
        external_data=False,
        verbose=False,
    )
    print(f"wrote {path}")


if __name__ == "__main__":
    main()
