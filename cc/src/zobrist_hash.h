#pragma once

#include <array>
#include <cstdint>

namespace bg {

using ZobristTableType = std::array<uint64_t, 32>;

inline int32_t zobrist_index(int32_t index) {
  return index;
}

const ZobristTableType &get_zobrist_table();

}  // namespace bg
