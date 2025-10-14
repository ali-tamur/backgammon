#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string_view>
#include <vector>

#ifdef PROJECT_GAMES_DIR
static constexpr const char* PROJECT_ROOT = PROJECT_GAMES_DIR;
#else
static constexpr const char* PROJECT_ROOT = "";
#endif

static constexpr size_t BOARD_SIZE = 28;
using Board = std::array<int8_t, BOARD_SIZE>;
auto white_move(const Board &board, int32_t d1, int32_t d2)
    -> std::vector<Board>;
auto black_move(const Board &board, int32_t d1, int32_t d2)
    -> std::vector<Board>;

namespace bg {

static constexpr int32_t COLLECT_START = 18;
static constexpr int32_t COLLECT_END = 24;
static constexpr int32_t WHITE_BAR = 24;
static constexpr int32_t BLACK_BAR = 25;
static constexpr int32_t WHITE_BAG = 26;
static constexpr int32_t BLACK_BAG = 27;

static constexpr int32_t ON_BAR = 0;
static constexpr int32_t NORMAL = 1;
static constexpr int32_t COLLECTING = 2;

struct WhiteConfig {
  static constexpr int32_t PLAYER_BAR = WHITE_BAR;
  static constexpr int32_t OPPONENT_BAR = BLACK_BAR;
  static constexpr int32_t PLAYER_BAG = WHITE_BAG;
  static constexpr int32_t OPPONENT_BAG = BLACK_BAG;
  static constexpr int8_t SIGN = 1;
  static constexpr int32_t HOME_START = COLLECT_START;
  static constexpr int32_t HOME_END = COLLECT_END;
  static constexpr int32_t LAND_START = 0;
  static constexpr int32_t EXACT_COLLECT = COLLECT_END;
  static constexpr std::string_view COLOR = "white";
  static constexpr auto is_valid_dst(int32_t value) -> bool {
    return value >= -1;
  }
  static constexpr auto apply_dice(int32_t src_index, int32_t dice) -> int32_t {
    return src_index + dice;
  }
  static constexpr auto is_inboard(int32_t dst_index) -> bool {
    return dst_index < HOME_END;
  }
  static constexpr auto move_to_home(int32_t src_index, int32_t dst_index)
      -> bool {
    return src_index < HOME_START && dst_index >= HOME_START;
  }
  static constexpr auto is_already_analyzed(int32_t last_cell, int32_t now_cell)
      -> bool {
    return now_cell < last_cell;
  }
};

struct BlackConfig {
  static constexpr int32_t PLAYER_BAR = BLACK_BAR;
  static constexpr int32_t OPPONENT_BAR = WHITE_BAR;
  static constexpr int32_t PLAYER_BAG = BLACK_BAG;
  static constexpr int32_t OPPONENT_BAG = WHITE_BAG;
  static constexpr int8_t SIGN = -1;
  static constexpr int32_t HOME_START = 0;
  static constexpr int32_t HOME_END = 6;
  static constexpr int32_t LAND_START = COLLECT_END - 1;
  static constexpr int32_t EXACT_COLLECT = -1;
  static constexpr std::string_view COLOR = "black";
  static constexpr auto is_valid_dst(int32_t value) -> bool {
    return value <= 1;
  }
  static constexpr auto apply_dice(int32_t src_index, int32_t dice) -> int32_t {
    return src_index - dice;
  }
  static constexpr auto is_inboard(int32_t dst_index) -> bool {
    return dst_index >= 0;
  }
  static constexpr auto move_to_home(int32_t src_index, int32_t dst_index)
      -> bool {
    return src_index >= HOME_END && dst_index < HOME_END;
  }
  static constexpr auto is_already_analyzed(int32_t last_cell, int32_t now_cell)
      -> bool {
    return now_cell > last_cell;
  }
};

static constexpr Board GAME_START = {2, 0, 0,  0,  0, -5, 0, -3, 0, 0,
                                     0, 5, -5, 0,  0, 0,  3, 0,  5, 0,
                                     0, 0, 0,  -2, 0, 0,  0, 0};

static constexpr int32_t NUM_STONES = 15;

class Dice;
using MOVE = std::function<Board(const Board &board, Dice)>;

static constexpr float DEFAULT_TEMPERATURE = 0.1;

} // namespace bg
