#pragma once

#include <array>
#include <cstdint>

namespace bg {

using ZobristTableType = std::array<uint32_t, 32>;
auto get_zobrist_table() -> const ZobristTableType &;

} // namespace bg
