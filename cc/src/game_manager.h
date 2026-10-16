#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "configs.h"
#include "dice.h"
#include "logger.h"
#include "player.h"

namespace bg {

enum class FirstMove : uint8_t { Always_White, Always_Black, Alternate };

class GameManager {
 public:
  GameManager(MOVE white, MOVE black, SharedRng rng, uint64_t seed, LogWriter logger,
              int32_t n_games, FirstMove first_move, bool is_interactive, bool print_score)
      : white_{std::move(white)},
        black_(std::move(black)),
        logger_(std::move(logger)),
        n_games_(n_games),
        first_move_(first_move),
        is_interactive_(is_interactive),
        print_score_(print_score),
        rng_(rng),
        seed_(seed),
        dice_throw_(std::move(rng)) {}

  // Plays game number `game_index` of the match; the same seed and index
  // always give the same game.
  auto run_once(LogWriter* logger, int32_t game_index) -> int32_t;
  auto run() -> std::tuple<int32_t, int32_t>;
  [[nodiscard]] auto seed() const -> uint64_t { return seed_; }

 private:
  const MOVE white_;
  const MOVE black_;
  LogWriter logger_;
  const int32_t n_games_;
  const FirstMove first_move_;
  const bool is_interactive_;
  const bool print_score_;
  SharedRng rng_;
  const uint64_t seed_;
  DiceThrow dice_throw_;
};

struct GameOptions {
  // "random" or path to an ONNX model.
  std::string white_model;

  // "random" or path to an ONNX model.
  std::string black_model;

  // Exploration of nn players: 0 always picks the best move.
  float white_temperature = DEFAULT_TEMPERATURE;
  float black_temperature = DEFAULT_TEMPERATURE;

  bool is_interactive;
  bool print_score;

  // leave empty iff there'll be no log.
  std::string log_path;

  int32_t n_games;

  FirstMove first_move;

  bool is_unittest = false;

  // Match seed; a fresh random one when unset (see GameManager::seed()).
  std::optional<uint64_t> seed;
};

auto game_manager_factory(const GameOptions& options) -> GameManager;

// One turn. The board and the result are in the fixed frame (white moves
// towards index 23); `player` sees the board from the mover's side. Returns the
// board after the move and the game result: 0 if the game goes on, otherwise
// +1..3 for a white win or -1..-3 for a black win.
auto play_turn(const MOVE& player, const Board& board, Dice dice, bool white_moves)
    -> std::pair<Board, int32_t>;

}  // namespace bg