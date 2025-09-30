#ifndef BG_CONFIGS_H_
#define BG_CONFIGS_H_

#include <array>
#include <cstdint>

namespace bg {

using Board = std::array<int32_t, 28>;

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
  static constexpr int32_t PLAYER_BAG = WHITE_BAR;
  static constexpr int32_t OPPONENT_BAG = BLACK_BAG;
  static constexpr int32_t SIGN = 1;
  static constexpr int32_t HOME_START = COLLECT_START;
  static constexpr int32_t HOME_END = COLLECT_END;
};

struct BlackConfig {
  static constexpr int32_t PLAYER_BAR = BLACK_BAR;
  static constexpr int32_t OPPONENT_BAR = WHITE_BAR;
  static constexpr int32_t PLAYER_BAG = BLACK_BAG;
  static constexpr int32_t OPPONENT_BAG = WHITE_BAG;
  static constexpr int32_t SIGN = -1;
  static constexpr int32_t HOME_START = 0;
  static constexpr int32_t HOME_END = 6;
};

}  // namespace bg

#endif  // BG_CONFIGS_H_
