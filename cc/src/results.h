#pragma once

#include <unordered_set>

#include "configs.h"
#include "state.h"
#include "state_hash.h"

namespace bg {

template <typename Config> class KeepResults {
public:
  void add(State<Config> p) {
    auto this_size = p.dice.size();
    if (n_dice == -1 || this_size < n_dice) {
      n_dice = this_size;
      results.clear();
      results.emplace(p);
    } else if (this_size == n_dice) {
      results.emplace(p);
    }
  }

  auto get_results() -> std::vector<State<Config>> {
    return std::vector<State<Config>>(results.begin(), results.end());
  }

private:
  std::unordered_set<State<Config>, std::hash<State<Config>>,
                     std::equal_to<State<Config>>>
      results{};
  std::int32_t n_dice = -1;
};

} // namespace bg
