#pragma once

#include "configs.h"
#include "state.h"

namespace std {
template <>
struct hash<::bg::State> {
  auto operator()(const ::bg::State& state) const -> size_t {
    return static_cast<size_t>(state.hash_);
  }
};

template <>
struct equal_to<::bg::State> {
  auto operator()(const ::bg::State& a, const ::bg::State& b) const -> bool { return a.b == b.b; }
};

}  // namespace std
