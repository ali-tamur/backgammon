#include <cassert>
#include <onnxruntime_cxx_api.h>
#include <string>

#include "model.h"

namespace bg {
Model::Model(const std::string &model_path)
    : env_{ORT_LOGGING_LEVEL_WARNING, "backgammon"}, 
      session_{env_, model_path.c_str(), Ort::SessionOptions{}} {
  Ort::SessionOptions session_options;
  session_options.SetIntraOpNumThreads(4);

  auto input_name_ptr = session_.GetInputNameAllocated(0, allocator_);
  input_name_ = input_name_ptr.get();

  auto output_name_ptr = session_.GetOutputNameAllocated(0, allocator_);
  output_name_ = output_name_ptr.get();

  auto input_info = session_.GetInputTypeInfo(0);
  auto tensor_info = input_info.GetTensorTypeAndShapeInfo();
  input_shape_ = tensor_info.GetShape();
}

std::string Model::GetInputName() const { return input_name_; }

std::string Model::GetOutputName() const { return output_name_; }

std::vector<int64_t> Model::GetInputShape() const { return input_shape_; }

std::vector<float> Model::Predict(const std::vector<NN_INPUT> &boards) {
  assert (boards.size() == 250);

  auto memory_info =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  std::vector<int64_t> input_shape{static_cast<long>(boards.size()), 250};

  std::vector<float> flattened;
  flattened.reserve(boards.size() * 250);
  for (const auto& board : boards) {
      flattened.insert(flattened.end(), board.begin(), board.end());
  }

  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
      memory_info,
      flattened.data(),
      flattened.size(),
      input_shape.data(),
      input_shape.size()
  );


  const char *input_names[] = {input_name_.c_str()};
  const char *output_names[] = {output_name_.c_str()};

  auto output_tensors =
      session_.Run(Ort::RunOptions{nullptr}, // Options (nullptr = default)
                   input_names,              // Input names
                   &input_tensor,            // Input tensors
                   1,                        // Number of inputs
                   output_names,             // Output names
                   1                         // Number of outputs
      );

  float *output_data = output_tensors[0].GetTensorMutableData<float>();
  size_t output_size =
      output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
    assert(output_size == boards.size()); 
    return std::vector<float>(output_data, output_data + output_size);
}

class OneHotEncoding {
public:
  OneHotEncoding(size_t size) : a_(size, 0.0f){} 

  void add(int32_t value, int32_t max_value, bool can_be_negative) {
    int32_t advance = can_be_negative ? 2 * max_value + 1 : max_value + 1;
    value = std::min(value, max_value);
    value = std::max(value, -max_value);
    if (value < 0) {
      value += max_value;
    }
    a_[index_ + value] = 1.0;
    index_ += advance;
  }

  std::vector<float> &&get() { return std::move(a_); }

private:
  std::vector<float> a_;
  int32_t index_ = 0;
};

NN_INPUT board_to_nn_input(const Board &board) {
  auto onehot = OneHotEncoding(250);
  for (int32_t i = 0; i < bg::COLLECT_END; ++i) {
    onehot.add(board[i], 4, true);
  }
  onehot.add(board[WHITE_BAR], 4, false);
  onehot.add(-board[BLACK_BAR], 4, false);
  onehot.add(board[WHITE_BAG], 10, false);
  onehot.add(-board[BLACK_BAG], 10, false);
  return std::move(onehot.get());
}

} // namespace bg
