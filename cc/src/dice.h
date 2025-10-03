#pragma once

#include <algorithm>
#include <iostream>

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

  iterator begin() const;
  iterator end() const;

 private:
  int32_t d1_;
  int32_t d2_;
};

class Dice {
 public:
  Dice(int32_t dice) : dice_{dice} {}
  Dice(int32_t d1, int32_t d2)
      : dice_{d1 == d2 ? 1111 * d1
                       : (std::min(d1, d2) * 10 + std::max(d1, d2))} {}

  DiceRolls values(bool use_only_max) const;
  Dice use_dice(int32_t d) const;
  size_t size() const;

  int32_t dice_;

  friend std::ostream &operator<<(std::ostream &os, const Dice &move);
};

inline std::ostream &operator<<(std::ostream &os, const Dice &dice) {
  os << dice.dice_;
  return os;
}

}  // namespace bg

