#pragma once

#include "configs.h"

namespace bg {

class Bits {
 private:
  class BitIterator {
    static constexpr int32_t N_BITS = 32;

   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = int32_t;
    using difference_type = std::ptrdiff_t;

    explicit BitIterator(uint32_t n) { n_ = n; };
    auto operator*() const -> value_type { return __builtin_ctz(n_); }

    auto operator++() -> BitIterator& {
      n_ = n_ & (n_ - 1);
      return *this;
    }
    auto operator==(const BitIterator& other) const -> bool { return n_ == other.n_; }
    auto operator!=(const BitIterator& other) const -> bool { return n_ != other.n_; }

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
  friend auto begin(const Bits& b) -> iterator { return b.begin(); }
  friend auto end(const Bits& b) -> iterator { return b.end(); }
};

};  // namespace bg
