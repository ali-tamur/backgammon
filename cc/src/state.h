#ifndef BG_STATE_H_
#define BG_STATE_H_

#include <array>

#include "bits.h"
#include "configs.h"
#include "dice.h"

namespace bg {

template <typename Config>
static constexpr int32_t score(const Board &b) {
  if (b[Config::PLAYER_BAG] * Config::SIGN < 15) {
    return 0;
  }
  for (auto i = Config::HOME_START; i < Config::HOME_END; ++i) {
    if (b[i] * Config::SIGN < 0) {
      return 3 * Config::SIGN;
    }
  }
  if (b[Config::OPPONENT_BAR] * Config::SIGN < 0) {
    return 3 * Config::SIGN;
  }
  return b[Config::OPPONENT_BAG] == 0 ? Config::SIGN * 2 : Config::SIGN * 1;
}

template <typename Config>
struct State {
 public:
  const Board b;
  const Dice dice;
  const Bits bits;
  const int32_t homestones;

  State(Board board, int32_t dice_value)
      : b{board},
        dice{Dice(dice_value)},
        bits{get_bits(board)},
        homestones{get_homestones(b)} {}

  int32_t phase() const {
    if (b[Config::PLAYER_BAR] != 0) {
      return ON_BAR;
    };
    if (Config::SIGN * (homestones + b[Config::PLAYER_BAG]) == 15) {
      return COLLECTING;
    }
    return NORMAL;
  };

  State update_board(int32_t src, int32_t dst, int32_t used_dice) const {
    Board new_b = b;
    auto new_homestones = homestones;
    auto new_bits = bits;
    new_b[src] -= Config::SIGN;
    if (new_b[src] == 0) {
      new_bits.unset(src);
    }
    if (new_b[dst] == -Config::SIGN) {
      new_b[BLACK_BAR] -= Config::SIGN;
      new_b[dst] = Config::SIGN;
    } else {
      new_b[dst] += Config::SIGN;
    }
    if (src >= Config::HOME_START && src < Config::HOME_END) {
      new_homestones += Config::SIGN;
    }
    if (dst == Config::PLAYER_BAG) {
      new_homestones -= Config::SIGN;
    } else {
      new_bits.set(dst);
    }
    auto new_dice = dice.use_dice(used_dice);
    return State(new_b, new_dice, new_bits, new_homestones);
  };

  int32_t score() const {
    if (b[WHITE_BAG] < 15 and b[BLACK_BAG] > -15) {
      return 0;
    }
    return ::bg::score<WhiteConfig>(b) + ::bg::score<BlackConfig>(b);
  }

 private:
  State(Board board, Dice dice, Bits bits, int32_t homestones)
      : b{board}, dice{dice}, bits{bits}, homestones{homestones} {}
  int32_t get_homestones(const Board &b) const {
    int32_t total = 0;
    for (int32_t i = Config::HOME_START; i < Config::HOME_END; ++i) {
      if (b[i] * Config::SIGN > 0) {
        total += b[i];
      };
    }
    return total;
  }
  Bits get_bits(const Board &b) const {
    Bits bits = Bits();
    for (int32_t i = 0; i < COLLECT_END; ++i) {
      if (b[i] * Config::SIGN > 0) {
        bits.set(i);
      }
    }
    return Bits(bits);
  };
};
}  // namespace bg

#endif  // BG_STATE_H_
