#pragma once

#include <cstdint>

#include "py_interface.h"  // IWYU pragma: keep

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
  static constexpr bool is_valid_dst(int32_t value) { return value >= -1; }
  static constexpr int32_t apply_dice(int32_t src_index, int32_t dice) {
    return src_index + dice;
  }
  static constexpr bool is_inboard(int32_t dst_index) {
    return dst_index < HOME_END;
  }
  static constexpr bool move_to_home(int32_t src_index, int32_t dst_index) {
    return src_index < HOME_START && dst_index >= HOME_START;
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
  static constexpr bool is_valid_dst(int32_t value) { return value <= 1; }
  static constexpr int32_t apply_dice(int32_t src_index, int32_t dice) {
    return src_index - dice;
  }
  static constexpr bool is_inboard(int32_t dst_index) { return dst_index >= 0; }
  static constexpr bool move_to_home(int32_t src_index, int32_t dst_index) {
    return src_index >= HOME_END && dst_index < HOME_END;
  }
};

}  // namespace bg
