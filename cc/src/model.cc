#include "model.h"

#include <onnxruntime_cxx_api.h>

#include <array>
#include <cassert>
#include <ranges>
#include <string>

namespace bg {

static constexpr int32_t CELL_LIMIT = 4;
static constexpr int32_t BAG_LIMIT = 10;

static constexpr size_t NN_INPUT_SIZE =
    (COLLECT_END * ((2 * CELL_LIMIT) + 1)) +  // Each board cell: [-4, 4]
    (CELL_LIMIT + 1) +                        // WHITE_BAR: [0, 4]
    (CELL_LIMIT + 1) +                        // BLACK_BAR: [0, 4]
    (BAG_LIMIT + 1) +                         // WHITE_BAG: [0, 10]
    (BAG_LIMIT + 1);                          // BLACK_BAG: [0, 10]

// One thread: a move evaluates a few dozen positions, too little work to share
// (measured: 4 threads or ONNX Runtime's default are 10-20% slower). Matches
// can still use all cores by running games in parallel processes.
static auto session_options() -> Ort::SessionOptions {
  Ort::SessionOptions options;
  options.SetIntraOpNumThreads(1);
  return options;
}

Model::Model(const std::string& model_path)
    : env_{ORT_LOGGING_LEVEL_WARNING, "backgammon"},
      session_{env_, model_path.c_str(), session_options()} {
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

auto Model::Predict(const std::vector<NN_INPUT>& boards) -> std::vector<Score> {
  auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

  std::vector<int64_t> input_shape{static_cast<int64_t>(boards.size()), NN_INPUT_SIZE};

  std::vector<float> flattened;
  flattened.reserve(boards.size() * NN_INPUT_SIZE);
  for (const auto& board : boards) {
    flattened.insert(flattened.end(), board.begin(), board.end());
  }

  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
      memory_info, flattened.data(), flattened.size(), input_shape.data(), input_shape.size());

  const std::array<const char*, 1> input_names = {input_name_.c_str()};
  const std::array<const char*, 1> output_names = {output_name_.c_str()};

  auto output_tensors = session_.Run(Ort::RunOptions{nullptr}, input_names.data(), &input_tensor,
                                     input_names.size(), output_names.data(), output_names.size());

  auto* output_data = output_tensors[0].GetTensorMutableData<float>();
  size_t output_size = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
  assert(output_size == boards.size() * 4);
  std::vector<Score> results;
  results.reserve(boards.size());
  for (size_t i = 0; i < output_size; i += 4) {
    results.emplace_back(Score{.white_win = output_data[i],
                               .white_gammon = output_data[i + 1],
                               .black_win = output_data[i + 2],
                               .black_gammon = output_data[i + 3]});
  }
  return results;
}

class OneHotEncoding {
 public:
  explicit OneHotEncoding() : a_(NN_INPUT_SIZE, 0.0F) {}

  // Sets one of the group's slots: [-max, max] -> 0 .. 2 * max if the value
  // can be negative, [0, max] -> 0 .. max otherwise. Values outside the range
  // are clipped to it.
  void add(int32_t value, int32_t max_value, bool can_be_negative) {
    const int32_t min_value = can_be_negative ? -max_value : 0;
    const int32_t slot = std::clamp(value, min_value, max_value) - min_value;
    a_[index_ + slot] = 1.0;
    index_ += max_value - min_value + 1;
  }

  auto get() -> std::vector<float>&& { return std::move(a_); }

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
auto board_to_nn_input(const Board& board) -> NN_INPUT {
  return board_to_nn_input_impl([&](int i) -> int8_t { return board[i]; });
}

auto position_to_model_input(const Position& position) -> NN_INPUT {
  return board_to_nn_input_impl([&](int i) -> int32_t { return position.cell(i); });
}

}  // namespace bg
