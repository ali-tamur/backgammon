#ifndef BG_MOVES_H_
#define BG_MOVES_H_

#include <array>
#include <numeric>
#include <vector>

#include "absl/strings/string_view.h"

#include "bits.h"
#include "configs.h"
#include "dice.h"
#include "results.h"
#include "state.h"

namespace bg {

template <typename Config>
std::vector<State<Config>> moves_on_bar(const State<Config> &p) {
  auto result = std::vector<State<Config>>{};
  for (auto d : p.dice.values(false)) {
    if (p.b[d - 1] >= -1) {
      result.push_back(p.update_board(WHITE_BAR, d - 1, d));
    }
  }
  return result;
}

template <typename Config>
class Move {
 public:
  std::vector<State<Config>> generate(Board board, int32_t dice) {
    auto result = KeepResults<Config>();
    auto states = std::vector<State<Config>>{State<Config>(board, dice)};
    int32_t i = 0;
    while (i < states.size()) {
      auto &p = states[i++];
      if (p.dice.size() == 0) {
        result.add(p);
      }
      auto phase = p.phase();
      if (phase == ON_BAR) {
        auto landings = moves_on_bar(p);
        if (landings.size() == 0) {
          result.add(p);
          continue;
        }
        for (auto &new_p : landings) {
          states.emplace_back(std::move(new_p));
        }
        continue;
      }
      for (auto d : p.dice.values(phase == COLLECTING)) {
        for (auto src_index = p.bits.begin(); src_index != p.bits.end();
             ++src_index) {
          if (phase == COLLECTING && (*src_index) + d >= COLLECT_END) {
            auto new_p = p.update_board(*src_index, WHITE_BAG, d);
            if (new_p.score() != 0) {
              return {new_p};
            }
            states.emplace_back(new_p);
            break;
          }
        }
      }
    }

    return result.get_results();
  };
};

}  // namespace bg
#endif  // BG_MOVES_H_
