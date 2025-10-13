#pragma once

#include <type_traits>

#include "configs.h"

namespace bg {

template <typename Config> class Bits {
  static_assert(std::is_same_v<Config, WhiteConfig> ||
                std::is_same_v<Config, BlackConfig>);

private:
  class BitIterator {
    static constexpr int32_t N_BITS = 32;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int32_t;
    using difference_type = std::ptrdiff_t;

    explicit BitIterator(uint32_t n) { n_ = n; };
    auto operator*() const -> value_type {
      if constexpr (std::is_same_v<Config, WhiteConfig>) {
        return __builtin_ctz(n_);
      } else if constexpr (std::is_same_v<Config, BlackConfig>) {
        return N_BITS - 1 - __builtin_clz(n_);
      }
    }

    auto operator++() -> BitIterator & {
      if constexpr (std::is_same_v<Config, WhiteConfig>) {
        n_ = n_ & (n_ - 1);
        return *this;
      } else if constexpr (std::is_same_v<Config, BlackConfig>) {
        n_ = n_ ^ (1U << (N_BITS - 1 - __builtin_clz(n_)));
        return *this;
      }
    }
    auto operator==(const BitIterator &other) const -> bool {
      return n_ == other.n_;
    }
    auto operator!=(const BitIterator &other) const -> bool {
      return n_ != other.n_;
    }

  private:
    uint32_t n_;
  };

public:
  using iterator = BitIterator;
  void set(size_t index) { n_ |= (1U << index); }
  void unset(size_t index) { n_ &= ~(1U << index); }
  [[nodiscard]] auto begin() const -> iterator { return BitIterator(n_); }
  [[nodiscard]] auto end() const -> iterator { return BitIterator(0); }

private:
  uint32_t n_ = 0;
  friend auto begin(const Bits &b) -> iterator { return b.begin(); }
  friend auto end(const Bits &b) -> iterator { return b.end(); }
};

}; // namespace bg
