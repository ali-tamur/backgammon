#include "moves.h"

#include <algorithm>

#include "configs.h"

namespace bg {
auto white_move(const Board &board, int32_t d1, int32_t d2) -> std::vector<Board> {
  const auto &states = generate<WhiteConfig>(board, Dice(d1, d2));
  std::vector<Board> result{};
  std::ranges::transform(states, std::back_inserter(result),
                         [](const auto &state) -> auto { return state.get_board(); });
  return result;
}

auto black_move(const Board &board, int32_t d1, int32_t d2) -> std::vector<Board> {
  const auto &states = generate<BlackConfig>(board, Dice(d1, d2));
  std::vector<Board> result{};
  std::ranges::transform(states, std::back_inserter(result),
                         [](const auto &state) -> auto { return state.get_board(); });
  return result;
}

}  // namespace bg