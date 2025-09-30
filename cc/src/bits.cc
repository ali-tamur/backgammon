#include "bits.h"

namespace bg {

size_t BitIterator::operator*() const { return __builtin_ctz(n_); }

BitIterator& BitIterator::operator++() {
  n_ = n_ & (n_ - 1);
  return *this;
}

bool BitIterator::operator==(const BitIterator& other) const {
  return n_ == other.n_;
}

bool BitIterator::operator!=(const BitIterator& other) const {
  return n_ != other.n_;
}

void Bits::set(size_t index) { n_ |= (1U << index); }
void Bits::unset(size_t index) { n_ &= ~(1U << index); }

Bits::iterator Bits::begin() const { return BitIterator(n_); }

Bits::iterator Bits::end() const { return BitIterator(0); }
};  // namespace bg
