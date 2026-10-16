#pragma once

// Turns game records into training examples for the network.
//
// Every turn of every game becomes one example: the position after the move,
// seen from the mover's side (black's positions are flipped, so the mover's
// stones are positive and move towards index 23), labelled with how the game
// ended for the mover. This is the position the network is asked about when
// that player chooses a move.

#include <cstdint>
#include <filesystem>
#include <vector>

#include "proto/game.pb.h"

namespace bg {

// The network's output classes, from the mover's side; the order matches the
// model's outputs ("white" there is the mover) and bg.Prediction.
enum Outcome : uint8_t {
  MOVER_SINGLE = 0,
  MOVER_GAMMON = 1,
  OPPONENT_SINGLE = 2,
  OPPONENT_GAMMON = 3
};

// Gammons and backgammons share a class; `points` keeps the difference.
auto outcome_for_mover(int32_t mover_points) -> Outcome;

// Column-wise: example i is boards[28 * i .. 28 * i + 27], labels[i], ...
struct TrainingExamples {
  std::vector<int8_t> boards;    // 28 cells each, mover's view
  std::vector<uint8_t> labels;   // Outcome
  std::vector<int8_t> points;    // the mover's result: +1..+3 won, -1..-3 lost
  std::vector<int16_t> turn;     // 0-based turn within the game
  std::vector<int16_t> n_turns;  // number of turns in the game
  std::vector<int32_t> game;     // index of the game in the input

  [[nodiscard]] auto size() const -> size_t { return labels.size(); }
};

void add_game(const Game& game, int32_t game_index, TrainingExamples* examples);

// Writes boards.npy, labels.npy, points.npy, turn.npy, n_turns.npy and
// game.npy into `dir`, which must exist.
void write_examples(const TrainingExamples& examples, const std::filesystem::path& dir);

}  // namespace bg
