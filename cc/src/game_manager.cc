#include "game_manager.h"

#include <chrono>
#include <iostream>

#include "assertion.h"
#include "configs.h"
#include "model.h"
#include "player.h"
#include "state.h"

namespace bg {

auto GameManager::run_once(LogWriter *logger) -> int32_t {
  auto now = std::chrono::system_clock::now();
  uint64_t game_id =
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  logger->start_game(game_id);
  std::array<MOVE, 2> players{white_, black_};
  int32_t this_score;
  int32_t on_turn = 0;
  Board board = GAME_START;

  while (true) {
    Dice dice = dice_throw_.throw_dice();
    if (is_interactive_) {
      std::cout << "placeholder";
    }

    board = (on_turn == 0) ? white_(board, dice) : black_(board, dice);
    logger->add_move(dice, board);

    if (on_turn == 0) {
      this_score = score<WhiteConfig>(board);
    } else {
      this_score = score<BlackConfig>(board);
    }
    if (this_score != 0) {
      break;
    }
    on_turn = 1 - on_turn;
  }
  logger->end_game(this_score);
  return this_score;
}

auto GameManager::run() -> std::tuple<int32_t, int32_t> {
  int32_t white_score = 0;
  int32_t black_score = 0;
  for (int32_t i = 0; i < n_games_; ++i) {
    int32_t this_score = run_once(&logger_);
    if (print_score_) {
      std::cout << i << "th game: " << this_score << "\n";
    }
    if (this_score > 0) {
      white_score += this_score;
    } else {
      black_score -= this_score;
    }
  }
  return std::make_tuple(white_score, black_score);
}

auto game_manager_factory(const GameOptions &options) -> GameManager {
  static constexpr size_t BUFFER_SIZE = 50000;
  std::mt19937 gen{std::random_device{}()};
  MOVE white_move;
  if (options.white_model == "random") {
    white_move = construct_random_player_move<WhiteConfig>(gen);
  } else {
    auto white_path =
        (std::filesystem::path(PROJECT_ROOT) / "models" / options.white_model).string();
    white_move = construct_nn_player_move<WhiteConfig>(gen, white_path, options.temperature);
  }
  MOVE black_move;
  if (options.black_model == "random") {
    black_move = construct_random_player_move<BlackConfig>(gen);
  } else {
    auto black_path =
        (std::filesystem::path(PROJECT_ROOT) / "models" / options.black_model).string();
    black_move = construct_nn_player_move<BlackConfig>(gen, black_path, options.temperature);
  }
  LogWriter logger;
  if (options.log_path != "" && !options.is_unittest) {
    auto logger_path = (std::filesystem::path(PROJECT_ROOT) / "games" / options.log_path).string();
    logger = LogWriter(logger_path, BUFFER_SIZE);
  } else if (options.is_unittest) {
    logger = LogWriter(options.log_path, BUFFER_SIZE);
  }
  return GameManager(white_move, black_move, logger, options.n_games, options.is_interactive,
                     options.print_score);
}

}  // namespace bg
