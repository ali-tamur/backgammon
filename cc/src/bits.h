#ifndef BG_BITS_H_
#define BG_BITS_H_

#include <numeric>

namespace bg {
class BitIterator {
 public:
  explicit BitIterator(uint32_t n) { n_ = n; };

  size_t operator*() const;
  BitIterator& operator++();
  bool operator==(const BitIterator& other) const;
  bool operator!=(const BitIterator& other) const;

 private:
  uint32_t n_;
};

class Bits {
 public:
  using iterator = BitIterator;
  using iterator_category = std::forward_iterator_tag;
  using value_type = int32_t;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type*;
  using reference = value_type&;

  Bits() : n_{0} {}
  void set(size_t index);
  void unset(size_t index);
  iterator begin() const;
  iterator end() const;

 private:
  uint32_t n_ = 0;
};

};  // namespace bg

#endif  // BG_BITS_H_
