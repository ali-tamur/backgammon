#pragma once

#include <algorithm>
#include <iostream>
#include <random>

namespace bg {

class DiceIterator {
public:
  using iterator = DiceIterator;
  using iterator_category = std::forward_iterator_tag;
  using value_type = int32_t;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type;

  explicit DiceIterator(int32_t d1, int32_t d2 = 0) : d1_{d1}, d2_{d2} {};

  int32_t operator*() const;
  iterator &operator++();
  bool operator==(const DiceIterator &other) const;
  bool operator!=(const DiceIterator &other) const;

private:
  int32_t d1_;
  int32_t d2_;
};

class DiceRolls {
public:
  using iterator = DiceIterator;
  explicit DiceRolls(int32_t d1, int32_t d2 = 0) : d1_{d1}, d2_{d2} {};

  [[nodiscard]] iterator begin() const;
  [[nodiscard]] static iterator end();

private:
  int32_t d1_;
  int32_t d2_;
};

class Dice {
public:
  Dice(int32_t dice) : dice_{dice} {}
  Dice(int32_t d1, int32_t d2)
      : dice_{d1 == d2 ? 1111 * d1
                       : ((std::min(d1, d2) * 10) + std::max(d1, d2))} {}

  [[nodiscard]] DiceRolls values(bool use_only_max) const;
  [[nodiscard]] Dice use_dice(int32_t d) const;

  [[nodiscard]] size_t size() const;

  int32_t dice_;

  friend auto operator<<(std::ostream &os, const Dice &dice) -> std::ostream &;
};

inline std::ostream &operator<<(std::ostream &os, const Dice &dice) {
  os << dice.dice_;
  return os;
}

class DiceThrow {
public:
  [[nodiscard]] Dice throw_dice();

private:
  std::mt19937 gen_{std::random_device{}()};
};

} // namespace bg
