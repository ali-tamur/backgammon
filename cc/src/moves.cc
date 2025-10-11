#include "moves.h"

#include "configs.h"
#include "py_interface.h"

namespace bg {
std::vector<Board> white_move(const Board &board, int32_t d1, int32_t d2) {
  const auto &states = generate<WhiteConfig>(board, Dice(d1, d2));
  std::vector<Board> result{};
  std::transform(states.begin(), states.end(), std::back_inserter(result),
                 [](const auto &state) { return state.get_board(); });
  return result;
}

std::vector<Board> black_move(const Board &board, int32_t d1, int32_t d2) {
  const auto &states = generate<BlackConfig>(board, Dice(d1, d2));
  std::vector<Board> result{};
  std::transform(states.begin(), states.end(), std::back_inserter(result),
                 [](const auto &state) { return state.get_board(); });
  return result;
}

} // namespace bg