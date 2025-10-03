#pragma once

#include <array>
#include <iostream>

#include "bits.h"
#include "configs.h"
#include "dice.h"
#include "zobrist_hash.h"

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
  static_assert(std::is_same_v<Config, WhiteConfig> ||
                std::is_same_v<Config, BlackConfig>);

 public:
  const Board b;
  const Dice dice;
  const Bits<Config> bits;
  const int32_t homestones;
  const uint64_t hash_;

  State(Board board, Dice dice)
      : b{board},
        dice{dice},
        bits{get_bits(board)},
        homestones{get_homestones(b)},
        hash_(calculate_hash(board)) {}

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
    uint64_t new_hash = hash_;
    update_(new_b, new_hash, src, -Config::SIGN);
    if (new_b[src] == 0) {
      new_bits.unset(src);
    }
    if (new_b[dst] == -Config::SIGN) {
      update_(new_b, new_hash, Config::OPPONENT_BAR, -Config::SIGN);
      update_(new_b, new_hash, dst, 2 * Config::SIGN);
    } else {
      update_(new_b, new_hash, dst, Config::SIGN);
    }
    if (Config::move_to_home(src, dst)) {
      new_homestones += Config::SIGN;
    }
    if (dst == Config::PLAYER_BAG) {
      new_homestones -= Config::SIGN;
    } else {
      new_bits.set(dst);
    }
    auto new_dice = dice.use_dice(used_dice);
    auto result = State(new_b, new_dice, new_bits, new_homestones, new_hash);
    return result;
  };

  int32_t score() const {
    if (b[WHITE_BAG] < 15 and b[BLACK_BAG] > -15) {
      return 0;
    }
    return ::bg::score<WhiteConfig>(b) + ::bg::score<BlackConfig>(b);
  }

  std::ostream &pretty_print(std::ostream &os) const {
    os << "[";
    for (int32_t i = 0; i < COLLECT_END; ++i) {
      os << b[i];
      if (i < COLLECT_END - 1) {
        os << ", ";
      }
    }
    os << "]";
    os << " d:" << dice << " h:" << homestones;
    return os;
  }

  Board get_board() const {
    return b;
  }

 private:
  State(Board board, Dice dice, Bits<Config> bits, int32_t homestones,
        uint64_t hash_)
      : b{board},
        dice{dice},
        bits{bits},
        homestones{homestones},
        hash_{hash_} {}

  int32_t get_homestones(const Board &b) const {
    int32_t total = 0;
    for (int32_t i = Config::HOME_START; i < Config::HOME_END; ++i) {
      if (b[i] * Config::SIGN > 0) {
        total += b[i];
      };
    }
    return total;
  }

  Bits<Config> get_bits(const Board &b) const {
    Bits<Config> bits = Bits<Config>();
    for (int32_t i = 0; i < COLLECT_END; ++i) {
      if (b[i] * Config::SIGN > 0) {
        bits.set(i);
      }
    }
    return Bits<Config>(bits);
  };

  inline void update_(Board &b, uint64_t &h, int32_t index,
                      int32_t delta) const {
    update_hash(h, index, b[index]);
    b[index] += delta;
    update_hash(h, index, b[index]);
  }

  inline void update_hash(uint64_t &hash, int32_t index, int32_t value) const {
    hash ^= get_zobrist_table()[zobrist_index(index, value)];
  }

  uint64_t calculate_hash(const Board &b) const {
    uint64_t h = 0;
    for (int32_t i = 0; i < 28; ++i) {
      update_hash(h, i, b[i]);
    }
    return h;
  }
};

template <typename Config>
std::ostream &operator<<(std::ostream &os, const State<Config> &state) {
  return state.pretty_print(os);
}

}  // namespace bg

