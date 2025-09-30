#ifndef BG_RESULTS_H_
#define BG_RESULTS_H_

#include "configs.h"
#include "state.h"

namespace bg {

template <typename Config>
class KeepResults {
 public:
  KeepResults() {
    results = std::vector<State<Config>>();
    n_dice = -1;
  }

  void add(State<Config> p) {
    auto this_size = p.dice.size();
    if (n_dice == -1 || this_size < n_dice) {
      n_dice = this_size;
      results.clear();
      results.emplace_back(p);
    } else if (this_size == n_dice) {
      results.push_back(p);
    }
  }

  std::vector<State<Config>> get_results() { return std::move(results); }

 private:
  std::vector<State<Config>> results;
  std::int32_t n_dice;
};

}  // namespace bg
#endif  // BG_RESULTS_H_
