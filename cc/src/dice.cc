#include "dice.h"

#include <random>

namespace bg {

int32_t DiceIterator::operator*() const { return d1_; }

DiceIterator &DiceIterator::operator++() {
  d1_ = d2_;
  d2_ = 0;
  return *this;
}

bool DiceIterator::operator==(const DiceIterator &other) const {
  return d1_ == other.d1_ && d2_ == other.d2_;
}

bool DiceIterator::operator!=(const DiceIterator &other) const {
  return d1_ != other.d1_ || d2_ != other.d2_;
}

DiceRolls::iterator DiceRolls::begin() const { return DiceIterator(d1_, d2_); }

DiceRolls::iterator DiceRolls::end() { return DiceIterator(0); }

DiceRolls Dice::values(bool use_only_max) const {
  auto d1 = dice_ % 10;
  auto d2 = (dice_ / 10) % 10;
  if (use_only_max || d1 == d2) {
    return DiceRolls(d1); // NOLINT(modernize-return-braced-init-list)
  }
  return DiceRolls(d1, d2); // NOLINT(modernize-return-braced-init-list)
}

auto Dice::use_dice(int32_t d) const -> Dice {
  if (d == dice_ % 10) {
    return Dice(dice_ / 10);
  }
  return Dice(dice_ % 10);
}

auto Dice::size() const -> size_t {
  return dice_ == 0     ? 0
         : dice_ < 10   ? 1
         : dice_ < 100  ? 2
         : dice_ < 1000 ? 3
                        : 4;
}
                        

[[nodiscard]] Dice DiceThrow::throw_dice() {
  std::uniform_int_distribution<int> dist_(1, 6);
  auto d1 = dist_(gen_);
  auto d2 = dist_(gen_);
  return Dice(d1, d2);
}

} // namespace bg
