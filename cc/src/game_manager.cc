#include "game_manager.h"

#include "configs.h"
#include "state.h"
#include <chrono>
#include <iostream>

namespace bg {

int32_t GameManager::run_once(LogWriter *logger) {
  auto now = std::chrono::system_clock::now();
  uint64_t game_id = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch())
                         .count();
  logger->start_game(game_id, options_.white_name, options_.black_name);
  std::array<MOVE, 2> players{white_, black_};
  int32_t this_score;
  int32_t on_turn = 0;
  Board board = GAME_START;
  while (true) {
    Dice dice = dice_throw_.throw_dice();
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

std::tuple<int32_t, int32_t> GameManager::run(int32_t n_times) {
  int32_t white_score = 0;
  int32_t black_score = 0;
  LogWriter logger{options_.db_path, !options_.will_log};
  for (int32_t i = 0; i < n_times; ++i) {
    int32_t this_score = run_once(&logger);
    if (options_.print_score) {
      std::cout << i << "th game: " << this_score << "\n";
    }
    if (this_score > 0) {
      white_score += this_score;
    } else {
      black_score -= this_score;
    }
  }
  //std::cout << n_times << " games played. Result: " << white_score << " - " << black_score << "\n";
  return std::make_tuple(white_score, black_score);
}

} // namespace bg