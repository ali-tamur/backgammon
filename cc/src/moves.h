#pragma once

#include <algorithm>
#include <vector>

#include "configs.h"
#include "dice.h"
#include "results.h"
#include "state.h"

namespace bg {

inline auto moves_on_bar(const State& p) -> std::vector<State> {
  auto result = std::vector<State>{};
  for (auto d : p.dice.values(false)) {
    auto land_index = Config::LAND_START + (d - 1);
    if (Config::is_valid_dst(p.b[land_index])) {
      result.emplace_back(p.update_board(Config::PLAYER_BAR, land_index, d));
    }
  }
  return result;
}

// If only one die of a non-double can be played, the larger one must be
// played when possible. `results` all use the same number of dice.
inline auto larger_die_rule(Dice dice, std::vector<State> results) -> std::vector<State> {
  if (dice.size() != 2 || results.empty() || results[0].dice.size() != 1) {
    return results;
  }
  // The die left over tells which one was played.
  const int32_t larger = *dice.values(true).begin();
  auto played_smaller = [&](const State& s) { return *s.dice.values(false).begin() == larger; };
  if (!std::ranges::all_of(results, played_smaller)) {
    std::erase_if(results, played_smaller);
  }
  return results;
}

inline auto generate(const Board& board, Dice dice) -> std::vector<State> {
  auto result = KeepResults();
  auto states = std::vector<std::tuple<State, size_t>>{};
  states.reserve(64);
  states.emplace_back(std::make_tuple(State(board, dice), Config::LAND_START));
  uint32_t i = 0;
  while (i < states.size()) {
    auto [p, last_cell] = states[i++];
    if (!p.dice.is_empty()) {
      auto phase = p.phase();
      if (phase == ON_BAR) {
        for (auto& new_p : moves_on_bar(p)) {
          states.push_back(std::make_tuple(new_p, Config::LAND_START));
        }
      } else {
        // Both dice orders, in every phase: only considering stones at or past
        // `last_cell` (below) relies on the other order to cover the rest.
        for (auto d : p.dice.values(false)) {
          bool first_move_in_collect = phase == COLLECTING;
          auto bits_copy = p.bits;
          for (auto src_index = bits_copy.begin(); src_index != bits_copy.end(); ++src_index) {
            if (Config::is_already_analyzed(last_cell, *src_index)) {
              first_move_in_collect = false;
              continue;
            }
            bool can_collect_wastefully = phase == COLLECTING && first_move_in_collect &&
                                          !Config::is_inboard(Config::apply_dice((*src_index), d));
            bool can_collect_exactly =
                phase == COLLECTING && Config::apply_dice((*src_index), d) == Config::EXACT_COLLECT;
            first_move_in_collect = false;
            if (can_collect_wastefully || can_collect_exactly) {
              auto new_p = p.update_board(*src_index, Config::PLAYER_BAG, d);
              if (new_p.score() != 0) {
                // All winning moves gets the same score, so no need to look
                // further.
                return {new_p};
              }
              states.push_back(std::make_tuple(new_p, *src_index));
              break;
            }
            auto dst_index = Config::apply_dice(*src_index, d);
            if (Config::is_inboard(dst_index) && Config::is_valid_dst(p.b[dst_index])) {
              auto new_p = p.update_board(*src_index, dst_index, d);
              states.push_back(std::make_tuple(new_p, *src_index));
            }
          }
        }
      }
    }
    result.add(p);
  }
  return larger_die_rule(dice, result.get_results());
}

}  // namespace bg
