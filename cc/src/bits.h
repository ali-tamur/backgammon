#pragma once

#include <numeric>
#include <type_traits>

#include "configs.h"

namespace bg {

template <typename Config> class Bits {
  static_assert(std::is_same_v<Config, WhiteConfig> ||
                std::is_same_v<Config, BlackConfig>);

private:
  class BitIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int32_t;
    using difference_type = std::ptrdiff_t;

    explicit BitIterator(uint32_t n) { n_ = n; };
    value_type operator*() const {
      if constexpr (std::is_same_v<Config, WhiteConfig>) {
        return __builtin_ctz(n_);
      } else if constexpr (std::is_same_v<Config, BlackConfig>) {
        return 31 - __builtin_clz(n_);
      }
    }

    BitIterator &operator++() {
      if constexpr (std::is_same_v<Config, WhiteConfig>) {
        n_ = n_ & (n_ - 1);
        return *this;
      } else if constexpr (std::is_same_v<Config, BlackConfig>) {
        n_ = n_ ^ (1U << (31 - __builtin_clz(n_)));
        return *this;
      }
    }
    bool operator==(const BitIterator &other) const { return n_ == other.n_; }
    bool operator!=(const BitIterator &other) const { return n_ != other.n_; }

  private:
    uint32_t n_;
  };

public:
  using iterator = BitIterator;
  void set(size_t index) { n_ |= (1U << index); }
  void unset(size_t index) { n_ &= ~(1U << index); }
  [[nodiscard]] iterator begin() const { return BitIterator(n_); }
  [[nodiscard]] iterator end() const { return BitIterator(0); }

private:
  uint32_t n_ = 0;
  friend iterator begin(const Bits &b) { return b.begin(); }
  friend iterator end(const Bits &b) { return b.end(); }
};

}; // namespace bg
