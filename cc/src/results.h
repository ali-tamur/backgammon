#pragma once

#include <unordered_set>

#include "configs.h"
#include "state.h"
#include "state_hash.h"

namespace bg {

class KeepResults {
 public:
  void add(State p) {
    auto this_size = p.dice.size();
    if (n_dice == -1 || this_size < n_dice) {
      n_dice = this_size;
      results.clear();
      results.emplace(p);
    } else if (this_size == n_dice) {
      results.emplace(p);
    }
  }

  auto get_results() -> std::vector<State> {
    return std::vector<State>(results.begin(), results.end());
  }

 private:
  std::unordered_set<State, std::hash<State>, std::equal_to<State>> results{};
  int32_t n_dice = -1;
};

}  // namespace bg
