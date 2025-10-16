#include "model.h"

#include <onnxruntime_cxx_api.h>

#include <cassert>
#include <ranges>
#include <string>

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

auto Model::GetInputName() const -> std::string { return input_name_; }

auto Model::GetOutputName() const -> std::string { return output_name_; }

auto Model::GetInputShape() const -> std::vector<int64_t> { return input_shape_; }

auto Model::Predict(const std::vector<NN_INPUT> &boards) -> std::vector<float> {
  assert(boards.size() == 250);

  auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  std::vector<int64_t> input_shape{static_cast<long>(boards.size()), 250};

  std::vector<float> flattened;
  flattened.reserve(boards.size() * 250);
  for (const auto &board : boards) {
    flattened.insert(flattened.end(), board.begin(), board.end());
  }

  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
      memory_info, flattened.data(), flattened.size(), input_shape.data(), input_shape.size());

  const char *input_names[] = {input_name_.c_str()};
  const char *output_names[] = {output_name_.c_str()};

  auto output_tensors = session_.Run(Ort::RunOptions{nullptr},  // Options (nullptr = default)
                                     input_names,               // Input names
                                     &input_tensor,             // Input tensors
                                     1,                         // Number of inputs
                                     output_names,              // Output names
                                     1                          // Number of outputs
  );

  float *output_data = output_tensors[0].GetTensorMutableData<float>();
  size_t output_size = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
  assert(output_size == boards.size());
  return std::vector<float>(output_data, output_data + output_size);
}

static constexpr int32_t CELL_LIMIT = 4;
static constexpr int32_t BAG_LIMIT = 10;

static constexpr size_t NN_INPUT_SIZE =
    (COLLECT_END * ((2 * CELL_LIMIT) + 1)) +  // Each board cell: [-4, 4]
    (CELL_LIMIT + 1) +                        // WHITE_BAR: [0, 4]
    (CELL_LIMIT + 1) +                        // BLACK_BAR: [0, 4]
    (BAG_LIMIT + 1) +                         // WHITE_BAG: [0, 10]
    (BAG_LIMIT + 1);                          // BLACK_BAG: [0, 10]

class OneHotEncoding {
 public:
  explicit OneHotEncoding() : a_(NN_INPUT_SIZE, 0.0F) {}

  void add(int32_t value, int32_t max_value, bool can_be_negative) {
    int32_t advance = can_be_negative ? (2 * max_value) + 1 : max_value + 1;
    value = std::min(value, max_value);
    value = std::max(value, -max_value);
    if (value < 0) {
      value += max_value;
    }
    a_[index_ + value] = 1.0;
    index_ += advance;
  }

  auto get() -> std::vector<float> && { return std::move(a_); }

 private:
  std::vector<float> a_;
  int32_t index_ = 0;
};

template <typename Accessor>
auto board_to_nn_input_impl(Accessor get_cell) -> NN_INPUT {
  auto onehot = OneHotEncoding();
  for (int32_t i = 0; i < COLLECT_END; ++i) {
    onehot.add(get_cell(i), CELL_LIMIT, true);
  }
  onehot.add(get_cell(WHITE_BAR), CELL_LIMIT, false);
  onehot.add(-get_cell(BLACK_BAR), CELL_LIMIT, false);
  onehot.add(get_cell(WHITE_BAG), BAG_LIMIT, false);
  onehot.add(-get_cell(BLACK_BAG), BAG_LIMIT, false);
  return onehot.get();
}

// Public overloads
auto board_to_nn_input(const Board &board) -> NN_INPUT {
  return board_to_nn_input_impl([&](int i) { return board[i]; });
}

auto position_to_nn_input(const Position &position) -> NN_INPUT {
  return board_to_nn_input_impl([&](int i) { return position.cell(i); });
}

}  // namespace bg
