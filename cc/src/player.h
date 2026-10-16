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
#include "proto/game.pb.h"

namespace bg {

enum PlayerTypes : int8_t { RANDOM, NN };

// Players always see the board from the mover's side (the mover's stones are
// positive and move towards index 23); the game manager flips it for black.
template <enum PlayerTypes>
class Player {
  auto move(const Board& board, Dice dice) -> Board;
};

template <>
class Player<RANDOM> {
 public:
  explicit Player(SharedRng r) : r_{std::move(r)} {}

  auto move(const Board& board, Dice dice) -> Board {
    const std::vector<State>& states = generate(board, dice);
    std::uniform_int_distribution<int> dist_(0, states.size() - 1);
    auto index = dist_(*r_);
    const State& choosen_state = states[index];
    return choosen_state.b;
  }

 private:
  SharedRng r_;
};

template <>
class Player<NN> {
 public:
  Player(SharedRng r, const std::string& model_path, float temperature = DEFAULT_TEMPERATURE)
      : r_{std::move(r)}, model_{Model(model_path)}, temperature_(temperature) {}

  auto move(const Board& board, Dice dice) -> Board {
    const std::vector<State>& states = generate(board, dice);
    std::vector<Board> boards;
    std::ranges::transform(states, std::back_inserter(boards),
                           [](const State& state) { return state.b; });
    if (boards.size() == 1) {
      return boards[0];
    }
    std::vector<NN_INPUT> nn_input;
    std::ranges::transform(boards, std::back_inserter(nn_input), &board_to_nn_input);
    std::vector<Model::Score> values = model_.Predict(nn_input);
    std::vector<float> weights;
    std::ranges::transform(values, std::back_inserter(weights), [this](Model::Score score) {
      return std::exp(expected_value(score) / temperature_);
    });
    std::discrete_distribution<> dist{weights.begin(), weights.end()};
    int32_t index = dist(*r_);
    return boards[index];
  }

  // From the mover's side: "white" in the model's output is the mover.
  [[nodiscard]] static auto expected_value(Model::Score score) -> float {
    return score.white_win - score.black_win + (2 * (score.white_gammon - score.black_gammon));
  }

 private:
  SharedRng r_;
  Model model_;
  float temperature_;
};

inline auto construct_random_player_move(SharedRng r) -> MOVE {
  auto alice = Player<RANDOM>(std::move(r));
  auto move = [alice](const Board& board, Dice dice) mutable -> auto {
    return alice.move(board, dice);
  };
  return move;
}

inline auto construct_nn_player_move(SharedRng r, const std::string& model_path, float temperature)
    -> MOVE {
  auto alice = std::make_shared<Player<NN>>(r, model_path, temperature);
  auto move = [alice](const Board& board, Dice dice) mutable -> auto {
    return alice->move(board, dice);
  };
  return move;
}

}  // namespace bg