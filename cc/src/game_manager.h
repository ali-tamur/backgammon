#pragma once

#include <string>
#include <utility>

#include "configs.h"
#include "dice.h"
#include "logger.h"
#include "player.h"

namespace bg {

class GameManager {
 public:
  GameManager(MOVE white, MOVE black, LogWriter logger, int32_t n_games, bool is_interactive,
              bool print_score)
      : white_{std::move(white)},
        black_(std::move(black)),
        logger_(std::move(logger)),
        n_games_(n_games),
        is_interactive_(is_interactive),
        print_score_(print_score),
        dice_throw_(DiceThrow()) {}

  auto run_once(LogWriter *logger) -> int32_t;
  auto run() -> std::tuple<int32_t, int32_t>;

 private:
  MOVE white_;
  MOVE black_;
  LogWriter logger_;
  int32_t n_games_;
  bool is_interactive_;
  bool print_score_;
  DiceThrow dice_throw_;
};

struct GameOptions {
  // "random" or path to the saved keras model.
  std::string white_model;

  // "random" or path to the saved keras model.
  std::string black_model;

  float temperature = DEFAULT_TEMPERATURE;

  bool is_interactive;
  bool print_score;

  // leave empty iff there'll be no log.
  std::string log_path;

  int32_t n_games;

  bool is_unittest = false;
};

auto game_manager_factory(const GameOptions &options) -> GameManager;

}  // namespace bg