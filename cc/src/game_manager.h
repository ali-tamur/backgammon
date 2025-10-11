#pragma once

#include "dice.h"
#include "logger.h"
#include "py_interface.h"
#include <filesystem>
#include <string>
#include <utility>

namespace bg {

struct GameOptions {
  std::string white_name;
  std::string black_name;
  bool is_interactive;
  bool print_score;
  bool will_log;
  std::filesystem::path db_path;
};

class GameManager {
public:
  using MOVE = std::function<Board(const Board &board, Dice)>;
  GameManager(MOVE white, MOVE black, DiceThrow dice_source,
              GameOptions options)
      : white_{std::move(white)}, black_{std::move(black)}, dice_throw_{dice_source},
        options_{std::move(options)} {}

  auto run_once(LogWriter *logger) -> int32_t;
  std::tuple<int32_t, int32_t> run(int32_t n_times);

private:
  MOVE white_;
  MOVE black_;
  DiceThrow dice_throw_;
  GameOptions options_;
};

} // namespace bg