#pragma once

#include <vector>
#include <array>
#include <cstdint>

using Board = std::array<int8_t, 28>;

namespace bg {

std::vector<Board> white_move(const Board& board, int32_t d1, int32_t d2);
std::vector<Board> black_move(const Board& board, int32_t d1, int32_t d2);
} // namespace bg

