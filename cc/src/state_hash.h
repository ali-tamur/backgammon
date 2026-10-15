#pragma once

#include "configs.h"
#include "state.h"

namespace std {
template <bg::Config C>
struct hash<::bg::State<C>> {
  auto operator()(const ::bg::State<C>& state) const -> size_t {
    return static_cast<size_t>(state.hash_);
  }
};

template <bg::Config C>
struct equal_to<::bg::State<C>> {
  auto operator()(const ::bg::State<C>& a, const ::bg::State<C>& b) const -> bool {
    return a.b == b.b;
  }
};

}  // namespace std
