#pragma once

#include "configs.h"
#include "state.h"

namespace std {
template <typename Config>
struct hash<::bg::State<Config>> {
  size_t operator()(const ::bg::State<Config> &state) const {
    return static_cast<size_t>(state.hash_);
  }
};

template <typename Config>
struct equal_to<::bg::State<Config>> {
  bool operator()(const ::bg::State<Config> &a,
                  const ::bg::State<Config> &b) const {
    return a.b == b.b;
  }
};

}  // namespace std

