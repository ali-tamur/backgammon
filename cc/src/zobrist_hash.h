#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>

namespace bg {

using ZobristTableType = std::array<uint64_t, 1024>;

inline int32_t zobrist_index(int32_t index, int32_t value) {
  return (index << 5) + value + 15;
}

const ZobristTableType &get_zobrist_table() {
  static const ZobristTableType INSTANCE = [] {
    size_t table_size = 1024;
    ZobristTableType table{};
    std::random_device rd{};
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<uint64_t> dist;
    for (size_t i = 0; i < table_size; ++i) {
      table[i] = dist(generator);
    }
    return table;
  }();
  return INSTANCE;
}

}  // namespace bg

