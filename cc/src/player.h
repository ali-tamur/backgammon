#pragma once

#include <onnxruntime_cxx_api.h>

#include <algorithm>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "configs.h"
#include "dice.h"
#include "model.h"
#include "moves.h"

namespace bg {

enum PlayerTypes : int8_t { RANDOM, NN };

template <enum PlayerTypes, Config C>
class Player {
  auto move(const Board &board, Dice dice) -> Board;
};

template <Config C>
class Player<RANDOM, C> {
 public:
  Player(std::mt19937 r) : r_{r} {}

  auto move(const Board &board, Dice dice) -> Board {
    const std::vector<State<C>> &states = generate<C>(board, dice);
    std::uniform_int_distribution<int> dist_(0, states.size() - 1);
    auto index = dist_(r_);
    const State<C> &choosen_state = states[index];
    return choosen_state.b;
  }

 private:
  std::mt19937 r_;
};

template <Config C>
class Player<NN, C> {
 public:
  Player(std::mt19937 r, const std::string &model_path, float temperature = DEFAULT_TEMPERATURE)
      : r_{r}, model_{Model(model_path)}, temperature_(temperature) {}

  auto move(const Board &board, Dice dice) -> Board {
    const std::vector<State<C>> &states = generate<C>(board, dice);
    std::vector<Board> boards;
    std::ranges::transform(states, std::back_inserter(boards),
                           [](const State<C> &state) { return state.b; });
    if (boards.size() == 1) {
      return boards[0];
    }
    std::vector<NN_INPUT> nn_input;
    std::ranges::transform(boards, std::back_inserter(nn_input), &board_to_nn_input);
    auto values = model_.Predict(nn_input);
    std::vector<float> weights;
    std::ranges::transform(values, std::back_inserter(weights),
                           [this](float val) { return std::exp(val / temperature_); });
    std::discrete_distribution<> dist{weights.begin(), weights.end()};
    int32_t index = dist(r_);
    return boards[index];
  }

 private:
  std::mt19937 r_;
  Model model_;
  float temperature_;
};

template <Config C>
auto construct_random_player_move(std::mt19937 r) -> MOVE {
  auto alice = Player<RANDOM, C>(r);
  auto move = [alice](const Board &board, Dice dice) mutable -> auto {
    return alice.move(board, dice);
  };
  return move;
}

template <Config C>
auto construct_nn_player_move(std::mt19937 r, std::string model_path, float temperature) -> MOVE {
  auto alice = std::make_shared<Player<NN, C>>(r, model_path, temperature);
  auto move = [alice](const Board &board, Dice dice) mutable -> auto {
    return alice->move(board, dice);
  };
  return move;
}

}  // namespace bg