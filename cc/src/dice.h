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

  auto operator*() const -> int32_t;
  auto operator++() -> iterator &;
  auto operator==(const DiceIterator &other) const -> bool;
  auto operator!=(const DiceIterator &other) const -> bool;

private:
  int32_t d1_;
  int32_t d2_;
};

class DiceRolls {
public:
  using iterator = DiceIterator;
  explicit DiceRolls(int32_t d1, int32_t d2 = 0) : d1_{d1}, d2_{d2} {};

  [[nodiscard]] auto begin() const -> iterator;
  [[nodiscard]] static auto end() -> iterator;

private:
  int32_t d1_;
  int32_t d2_;
};

class Dice {
public:
  explicit Dice(uint32_t dice) : dice_{dice} {}
  Dice(int32_t d1, int32_t d2);

  [[nodiscard]] auto values(bool use_only_max) const -> DiceRolls;
  [[nodiscard]] auto use_dice(int32_t d) const -> Dice;
  [[nodiscard]] auto size() const -> size_t;

  uint32_t dice_;

  friend auto operator<<(std::ostream &os, const Dice &dice) -> std::ostream &;
};

inline auto operator<<(std::ostream &os, const Dice &dice) -> std::ostream & {
  os << dice.dice_;
  return os;
}

class DiceThrow {
public:
  [[nodiscard]] auto throw_dice() -> Dice;

private:
  std::mt19937 gen_{std::random_device{}()};
};


} // namespace bg
