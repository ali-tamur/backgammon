#pragma once

#include "state.h"

namespace std {
template <typename Config>
struct hash<::bg::State<Config>> {
  auto operator()(const ::bg::State<Config> &state) const -> size_t {
    return static_cast<size_t>(state.hash_);
  }
};

template <typename Config>
struct equal_to<::bg::State<Config>> {
  auto operator()(const ::bg::State<Config> &a, const ::bg::State<Config> &b) const -> bool {
    return a.b == b.b;
  }
};

}  // namespace std
