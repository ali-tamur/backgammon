#pragma once

#include <iostream>
#include <onnxruntime_cxx_api.h>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "dice.h"
#include "model.h"
#include "moves.h"
#include "py_interface.h"

namespace bg {

enum PlayerTypes : int8_t { RANDOM, NN };

template <enum PlayerTypes, typename Config> class Player {
  auto move(const Board &board, Dice dice) -> Board;
};

template <typename Config> class Player<RANDOM, Config> {
public:
  Player(std::string name, std::mt19937 r) : name{std::move(name)}, r_{r} {}

  auto move(const Board &board, Dice dice) -> Board {
    const std::vector<State<Config>> &states = generate<Config>(board, dice);
    std::uniform_int_distribution<int> dist_(0, states.size() - 1);
    auto index = dist_(r_);
    const State<Config> &choosen_state = states[index];
    return choosen_state.b;
  }

  std::string name;

private:
  std::mt19937 r_;
};

template <typename Config> class Player<NN, Config> {
public:
  Player(std::string name, std::mt19937 r, const std::string &model_path)
      : name{std::move(name)}, r_{r}, model_{Model(model_path)} {}

  auto move(const Board &board, Dice dice) -> Board {
    const std::vector<State<Config>> &states = generate<Config>(board, dice);
    std::vector<Board> boards;
    std::transform(states.begin(), states.end(), std::back_inserter(boards),
                   [](const State<Config> &state) -> auto { return state.b; });
    if (boards.size() == 1) {
      return boards[0];
    }
    std::vector<NN_INPUT> nn_input;
    std::transform(boards.begin(), boards.end(), std::back_inserter(nn_input),
                   &board_to_nn_input);
    auto values = model_.Predict(nn_input);
    std::vector<float> weights;
    std::transform(values.begin(), values.end(), std::back_inserter(weights),
                   [this](float val) { return std::exp(val / temperature_); });
    std::discrete_distribution<> dist{weights.begin(), weights.end()};
    int32_t index = dist(r_);
    return boards[index];
  }

  std::string name;

private:
  std::mt19937 r_;
  Model model_;
  float temperature_ = 0.1;
};

} // namespace bg