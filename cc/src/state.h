#pragma once

#include <iostream>

#include "bits.h"
#include "configs.h"
#include "dice.h"
#include "zobrist_hash.h"

namespace bg {

template <typename Config> static constexpr int32_t score(const Board &b) {
  if (b[Config::PLAYER_BAG] * Config::SIGN < NUM_STONES) {
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

template <typename Config> struct State {
  static_assert(std::is_same_v<Config, WhiteConfig> ||
                std::is_same_v<Config, BlackConfig>);

public:
  Board b;
  Dice dice;
  Bits<Config> bits;
  int8_t homestones;
  uint64_t hash_;

  State(Board board, Dice dice)
      : b{board}, dice{dice}, bits{get_bits(board)},
        homestones{get_homestones(b)}, hash_(0) {}

  [[nodiscard]] auto phase() const -> int32_t {
    if (b[Config::PLAYER_BAR] != 0) {
      return ON_BAR;
    };
    if (Config::SIGN * (homestones + b[Config::PLAYER_BAG]) == NUM_STONES) {
      return COLLECTING;
    }
    return NORMAL;
  };

  // Updates the board, the bitarray, the hash and # homestones.
  [[nodiscard]] auto update_board(int32_t src, int32_t dst,
                                   int32_t used_dice) const -> State {
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
    return State(new_b, new_dice, new_bits, new_homestones, new_hash);
  };

  [[nodiscard]] auto score() const -> int32_t {
    if (b[WHITE_BAG] < NUM_STONES and b[BLACK_BAG] > -NUM_STONES) {
      return 0;
    }
    return ::bg::score<WhiteConfig>(b) + ::bg::score<BlackConfig>(b);
  }

  auto pretty_print(std::ostream &os) const -> std::ostream & {
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

  [[nodiscard]] auto get_board() const -> Board { return b; }

private:
  State(Board board, Dice dice, Bits<Config> bits, int8_t homestones,
        uint64_t hash_)
      : b{board}, dice{dice}, bits{bits}, homestones{homestones}, hash_{hash_} {
  }

  [[nodiscard]] auto get_homestones(const Board &b) const -> int8_t {
    int32_t total = 0;
    for (int32_t i = Config::HOME_START; i < Config::HOME_END; ++i) {
      if (b[i] * Config::SIGN > 0) {
        total += b[i];
      };
    }
    return static_cast<int8_t>(total);
  }

  [[nodiscard]] auto get_bits(const Board &b) const -> Bits<Config> {
    Bits<Config> bits = Bits<Config>();
    for (int32_t i = 0; i < COLLECT_END; ++i) {
      if (b[i] * Config::SIGN > 0) {
        bits.set(i);
      }
    }
    return bits;
  };

  void update_(Board &b, uint64_t &h, int32_t index, int8_t delta) const {
    update_hash(h, index);
    b[index] += delta;
  }

  void update_hash(uint64_t &hash, int32_t index) const {
    hash ^= get_zobrist_table()[index];
  }

};

template <typename Config>
std::ostream &operator<<(std::ostream &os, const State<Config> &state) {
  return state.pretty_print(os);
}

} // namespace bg
