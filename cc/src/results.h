#pragma once

#include <unordered_set>

#include "configs.h"
#include "state.h"
#include "state_hash.h"

namespace bg {

template <Config C>
class KeepResults {
 public:
  void add(State<C> p) {
    auto this_size = p.dice.size();
    if (n_dice == -1 || this_size < n_dice) {
      n_dice = this_size;
      results.clear();
      results.emplace(p);
    } else if (this_size == n_dice) {
      results.emplace(p);
    }
  }

  auto get_results() -> std::vector<State<C>> {
    return std::vector<State<C>>(results.begin(), results.end());
  }

 private:
  std::unordered_set<State<C>, std::hash<State<C>>, std::equal_to<State<C>>> results{};
  int32_t n_dice = -1;
};

}  // namespace bg
