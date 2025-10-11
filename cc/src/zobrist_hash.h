#pragma once

#include <array>
#include <cstdint>

namespace bg {

using ZobristTableType = std::array<uint32_t, 32>;

//inline auto zobrist_index(int32_t index) -> int32_t { return index; }

auto get_zobrist_table() -> const ZobristTableType &;

} // namespace bg
