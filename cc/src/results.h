#ifndef BG_RESULTS_H_
#define BG_RESULTS_H_

#include <unordered_set>

#include "configs.h"
#include "state.h"
#include "state_hash.h"

namespace bg {

template <typename Config>
class KeepResults {
 public:
  KeepResults() : results{}, n_dice{-1} {}

  void add(State<Config> p) {
    auto this_size = p.dice.size();
    if (n_dice == -1 || this_size < n_dice) {
      n_dice = this_size;
      results.clear();
      results.insert(p);
    } else if (this_size == n_dice) {
      results.insert(p);
    }
  }

  std::vector<State<Config>> get_results() {
    std::vector<State<Config>> r{};
    for (auto &elm : results) {
      r.push_back(elm);
    }
    return r;
  }

 private:
  std::unordered_set<State<Config>, std::hash<State<Config>>,
                     std::equal_to<State<Config>>>
      results;
  std::int32_t n_dice;
};

}  // namespace bg
#endif  // BG_RESULTS_H_
