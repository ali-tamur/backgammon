#pragma once

#include <onnxruntime_cxx_api.h>

#include <string>
#include <vector>

#include "configs.h"
#include "proto/game.pb.h"

namespace bg {

using NN_INPUT = std::vector<float>;

class Model {
 public:
  struct Score {
    float white_win;
    float white_gammon;
    float black_win;
    float black_gammon;
  };

  explicit Model(const std::string& model_path);

  [[nodiscard]] auto GetInputName() const -> std::string;
  [[nodiscard]] auto GetOutputName() const -> std::string;
  [[nodiscard]] auto GetInputShape() const -> std::vector<int64_t>;
  auto Predict(const std::vector<NN_INPUT>& boards) -> std::vector<Score>;

 private:
  Ort::Env env_;
  Ort::Session session_;
  Ort::AllocatorWithDefaultOptions allocator_;
  std::string input_name_;
  std::string output_name_;
  std::vector<int64_t> input_shape_;
};

// The network's 248 inputs: one-hot groups, in this order,
//   24 points:  9 slots each, stones -4..+4 (mover's positive), clipped
//   mover's bar, opponent's bar:  5 slots each, 0..4 stones, clipped
//   mover's bag, opponent's bag: 11 slots each, 0..10 stones, clipped
auto board_to_nn_input(const Board& board) -> NN_INPUT;
auto position_to_model_input(const Position& position) -> NN_INPUT;

}  // namespace bg
