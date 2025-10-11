#pragma once

#include "py_interface.h"
#include <onnxruntime_cxx_api.h>
#include <string>

namespace bg {

using NN_INPUT = std::vector<float>;

class Model {
public:
  Model(const std::string &model_path);

  std::string GetInputName() const;
  std::string GetOutputName() const;
  std::vector<int64_t> GetInputShape() const;
  std::vector<float> Predict(const std::vector<NN_INPUT> &boards);

private:
  Ort::Env env_;
  Ort::Session session_;
  Ort::AllocatorWithDefaultOptions allocator_;
  std::string input_name_;
  std::string output_name_;
  std::vector<int64_t> input_shape_;
};

auto board_to_nn_input(const Board &board) -> NN_INPUT;

} // namespace bg
