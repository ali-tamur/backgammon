#pragma once

#include <array>
#include <cstdint>

namespace bg {
using ZobristTableType = std::array<uint32_t, 32>;
extern const ZobristTableType ZOBRIST_TABLE;
}  // namespace bg
