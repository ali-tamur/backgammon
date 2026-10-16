#include "training_data.h"

#include <span>

#include "assertion.h"
#include "configs.h"
#include "npy.h"

namespace bg {

auto outcome_for_mover(int32_t mover_points) -> Outcome {
  ASSERT(mover_points != 0 && mover_points >= -3 && mover_points <= 3,
         "a finished game scores 1..3 points");
  if (mover_points > 0) {
    return mover_points == 1 ? MOVER_SINGLE : MOVER_GAMMON;
  }
  return mover_points == -1 ? OPPONENT_SINGLE : OPPONENT_GAMMON;
}

void add_game(const Game& game, int32_t game_index, TrainingExamples* examples) {
  auto n_turns = static_cast<int16_t>(game.turns_size());
  for (int16_t turn = 0; turn < n_turns; ++turn) {
    // Colours alternate, starting with the colour that moved first.
    bool white_moved = (turn % 2 == 0) == game.white_first();
    const auto& cells = game.turns(turn).after().cell();
    ASSERT(cells.size() == BOARD_SIZE, "a position has 28 cells");
    Board board{};
    for (size_t i = 0; i < BOARD_SIZE; ++i) {
      board[i] = static_cast<int8_t>(cells[static_cast<int32_t>(i)]);
    }
    if (!white_moved) {
      board = flip(board);
    }
    int32_t mover_points = white_moved ? game.final_score() : -game.final_score();
    examples->boards.insert(examples->boards.end(), board.begin(), board.end());
    examples->labels.push_back(outcome_for_mover(mover_points));
    examples->points.push_back(static_cast<int8_t>(mover_points));
    examples->turn.push_back(turn);
    examples->n_turns.push_back(n_turns);
    examples->game.push_back(game_index);
  }
}

void write_examples(const TrainingExamples& examples, const std::filesystem::path& dir) {
  size_t n = examples.size();
  write_npy<int8_t>(dir / "boards.npy", std::span(examples.boards), {n, BOARD_SIZE});
  write_npy<uint8_t>(dir / "labels.npy", std::span(examples.labels), {n});
  write_npy<int8_t>(dir / "points.npy", std::span(examples.points), {n});
  write_npy<int16_t>(dir / "turn.npy", std::span(examples.turn), {n});
  write_npy<int16_t>(dir / "n_turns.npy", std::span(examples.n_turns), {n});
  write_npy<int32_t>(dir / "game.npy", std::span(examples.game), {n});
}

}  // namespace bg
