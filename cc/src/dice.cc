#include "dice.h"

#include <cassert>
#include <random>

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {

static constexpr uint32_t BIT_MASK = 0b1111;
static constexpr size_t DICE_N = 6;
static constexpr size_t DICE_N2 = DICE_N * DICE_N;

auto init_dice_lookup() -> std::array<uint32_t, DICE_N2> {
  std::array<uint32_t, DICE_N2> result;
  for (int32_t d1_d2 = 0; d1_d2 < DICE_N2; ++d1_d2) {
    int32_t d1 = (d1_d2 / DICE_N);
    int32_t d2 = (d1_d2 % DICE_N);
    uint32_t value =
        (d1 == d2) ? BIT_MASK << (4 * d1) : (0b0001 << (4 * d1)) | (0b0001 << (4 * d2));
    result[d1_d2] = value;
  }
  return result;
}

static const std::array<uint32_t, DICE_N2> DICE_LOOKUP = init_dice_lookup();

static const std::array<int32_t, 4 * DICE_N> MAX_DICE_LOOKUP = {6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4,
                                                                3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1};

static const std::array<int32_t, 4 * DICE_N> MIN_DICE_LOOKUP = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
                                                                4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6};

auto init_dice_mask() -> std::array<uint32_t, DICE_N + 1> {
  std::array<uint32_t, DICE_N + 1> result;
  for (size_t d = 0; d < 6; ++d) {
    result[d + 1] = BIT_MASK << (4 * d);
  }
  return result;
}

static const std::array<uint32_t, DICE_N + 1> DICE_MASK = init_dice_mask();

[[nodiscard]] auto DiceThrow::throw_dice() -> Dice {
  std::uniform_int_distribution<int> dist(0, DICE_N2 - 1);
  return Dice(DICE_LOOKUP[dist(gen_)]);
}

auto DiceIterator::operator*() const -> int32_t { return d1_; }

auto DiceIterator::operator++() -> DiceIterator& {
  d1_ = d2_;
  d2_ = 0;
  return *this;
}

auto DiceIterator::operator==(const DiceIterator& other) const -> bool {
  return d1_ == other.d1_ && d2_ == other.d2_;
}

auto DiceIterator::operator!=(const DiceIterator& other) const -> bool {
  return d1_ != other.d1_ || d2_ != other.d2_;
}

auto DiceRolls::begin() const -> DiceRolls::iterator { return DiceIterator(d1_, d2_); }

auto DiceRolls::end() -> DiceRolls::iterator { return DiceIterator(0); }

Dice::Dice(int32_t d1, int32_t d2) { dice_ = DICE_LOOKUP[((d1 - 1) * DICE_N) + d2 - 1]; }

auto Dice::values(bool use_only_max) const -> DiceRolls {
  if (dice_ == 0) {
    return DiceRolls(0);
  }
  auto d1 = MAX_DICE_LOOKUP[__builtin_clz(dice_ << 8)];
  auto d2 = MIN_DICE_LOOKUP[__builtin_ctz(dice_)];
  return (use_only_max || (d1 == d2) ? DiceRolls(d1) : DiceRolls(d1, d2));
}

auto Dice::use_dice(int32_t d) const -> Dice {
  uint32_t masked = dice_ & DICE_MASK[d];
  auto reduced = masked & (masked - 1);
  auto result = (~DICE_MASK[d] & dice_) | reduced;
  return Dice(result);
}

}  // namespace bg
// NOLINTEND(readability-magic-numbers)
