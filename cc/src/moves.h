#pragma once

#include <vector>

#include "configs.h"
#include "dice.h"
#include "results.h"
#include "state.h"

namespace bg {

template <Config C>
auto moves_on_bar(const State<C>& p) -> std::vector<State<C>> {
  auto result = std::vector<State<C>>{};
  for (auto d : p.dice.values(false)) {
    auto land_index = C::LAND_START + (C::SIGN * (d - 1));
    if (C::is_valid_dst(p.b[land_index])) {
      result.emplace_back(p.update_board(C::PLAYER_BAR, land_index, d));
    }
  }
  return result;
}

template <Config C>
auto generate(const Board& board, Dice dice) -> std::vector<State<C>> {
  auto result = KeepResults<C>();
  auto states = std::vector<std::tuple<State<C>, size_t>>{};
  states.reserve(64);
  states.emplace_back(std::make_tuple(State<C>(board, dice), C::LAND_START));
  uint32_t i = 0;
  while (i < states.size()) {
    auto [p, last_cell] = states[i++];
    if (!p.dice.is_empty()) {
      auto phase = p.phase();
      if (phase == ON_BAR) {
        for (auto& new_p : moves_on_bar(p)) {
          states.push_back(std::make_tuple(new_p, C::LAND_START));
        }
      } else {
        // In the collecting phase we should consider the largest dice first.
        for (auto d : p.dice.values(phase == COLLECTING)) {
          bool first_move_in_collect = phase == COLLECTING;
          auto bits_copy = p.bits;
          for (auto src_index = bits_copy.begin(); src_index != bits_copy.end(); ++src_index) {
            if (C::is_already_analyzed(last_cell, *src_index)) {
              first_move_in_collect = false;
              continue;
            }
            bool can_collect_wastefully = phase == COLLECTING && first_move_in_collect &&
                                          !C::is_inboard(C::apply_dice((*src_index), d));
            bool can_collect_exactly =
                phase == COLLECTING && C::apply_dice((*src_index), d) == C::EXACT_COLLECT;
            first_move_in_collect = false;
            if (can_collect_wastefully || can_collect_exactly) {
              auto new_p = p.update_board(*src_index, C::PLAYER_BAG, d);
              if (new_p.score() != 0) {
                // All winning moves gets the same score, so no need to look
                // further.
                return {new_p};
              }
              states.push_back(std::make_tuple(new_p, *src_index));
              break;
            }
            auto dst_index = C::apply_dice(*src_index, d);
            if (C::is_inboard(dst_index) && C::is_valid_dst(p.b[dst_index])) {
              auto new_p = p.update_board(*src_index, dst_index, d);
              states.push_back(std::make_tuple(new_p, *src_index));
            }
          }
        }
      }
    }
    result.add(p);
  }
  return result.get_results();
};

}  // namespace bg
