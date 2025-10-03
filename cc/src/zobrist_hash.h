#pragma once

#include <array>
#include <cstdint>

namespace bg {

using ZobristTableType = std::array<uint64_t, 1024>;

inline int32_t zobrist_index(int32_t index, int32_t value) {
  return (index << 5) + value + 15;
}

const ZobristTableType &get_zobrist_table();

}  // namespace bg

