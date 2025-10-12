#pragma once

#include <array>
#include <cstdint>
#include <random>

namespace bg {
using ZobristTableType = std::array<uint32_t, 32>;
extern const ZobristTableType ZOBRIST_TABLE; 
} // namespace bg
