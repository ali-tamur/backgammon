#pragma once

#include <iostream>

#include "assertion.h"
#include "bits.h"
#include "configs.h"
#include "dice.h"
#include "zobrist_hash.h"

namespace bg {

template <Config C>
static constexpr int32_t score(const Board &b) {
  if (b[C::PLAYER_BAG] * C::SIGN < NUM_STONES) {
    return 0;
  }
  for (auto i = C::HOME_START; i < C::HOME_END; ++i) {
    if (b[i] * C::SIGN < 0) {
      return 3 * C::SIGN;
    }
  }
  if (b[C::OPPONENT_BAR] * C::SIGN < 0) {
    return 3 * C::SIGN;
  }
  return b[C::OPPONENT_BAG] == 0 ? C::SIGN * 2 : C::SIGN * 1;
}

template <Config C>
struct State {
 public:
  Board b;
  Dice dice;
  Bits<C> bits;
  int8_t homestones;
  uint64_t hash_;

  State(Board board, Dice dice)
      : b{board}, dice{dice}, bits{get_bits(board)}, homestones{get_homestones(b)}, hash_(0) {}

  [[nodiscard]] auto phase() const -> int32_t {
    if (b[C::PLAYER_BAR] != 0) {
      return ON_BAR;
    };
    if (C::SIGN * (homestones + b[C::PLAYER_BAG]) == NUM_STONES) {
      return COLLECTING;
    }
    return NORMAL;
  };

  // Updates the board, the bitarray, the hash and # homestones.
  [[nodiscard]] auto update_board(int32_t src, int32_t dst, int32_t used_dice) const -> State {
    Board new_b = b;
    auto new_homestones = homestones;
    auto new_bits = bits;
    uint64_t new_hash = hash_;
    update_(new_b, new_hash, src, -C::SIGN);
    if (new_b[src] == 0) {
      new_bits.unset(src);
    }
    if (new_b[dst] == -C::SIGN) {
      update_(new_b, new_hash, C::OPPONENT_BAR, -C::SIGN);
      update_(new_b, new_hash, dst, 2 * C::SIGN);
    } else {
      update_(new_b, new_hash, dst, C::SIGN);
    }
    if (C::move_to_home(src, dst)) {
      new_homestones += C::SIGN;
    }
    if (dst == C::PLAYER_BAG) {
      new_homestones -= C::SIGN;
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
  State(Board board, Dice dice, Bits<C> bits, int8_t homestones, uint64_t hash_)
      : b{board}, dice{dice}, bits{bits}, homestones{homestones}, hash_{hash_} {}

  [[nodiscard]] auto get_homestones(const Board &b) const -> int8_t {
    int32_t total = 0;
    for (int32_t i = C::HOME_START; i < C::HOME_END; ++i) {
      if (b[i] * C::SIGN > 0) {
        total += b[i];
      };
    }
    return static_cast<int8_t>(total);
  }

  [[nodiscard]] auto get_bits(const Board &b) const -> Bits<C> {
    Bits<C> bits = Bits<C>();
    for (int32_t i = 0; i < COLLECT_END; ++i) {
      if (b[i] * C::SIGN > 0) {
        bits.set(i);
      }
    }
    return bits;
  };

  void update_(Board &b, uint64_t &h, int32_t index, int8_t delta) const {
    update_hash(h, index);
    b[index] += delta;
  }

  void update_hash(uint64_t &hash, int32_t index) const { hash ^= ZOBRIST_TABLE[index]; }
};

template <Config C>
auto operator<<(std::ostream &os, const State<C> &state) -> std::ostream & {
  return state.pretty_print(os);
}

}  // namespace bg
