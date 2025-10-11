#pragma once

#include <vector>

#include "configs.h"
#include "dice.h"
#include "results.h"
#include "state.h"

namespace bg {

template <typename Config>
auto moves_on_bar(const State<Config> &p) -> std::vector<State<Config>> {
  auto result = std::vector<State<Config>>{};
  for (auto d : p.dice.values(false)) {
    auto land_index = Config::LAND_START + (Config::SIGN * (d - 1));
    if (Config::is_valid_dst(p.b[land_index])) {
      result.emplace_back(p.update_board(Config::PLAYER_BAR, land_index, d));
    }
  }
  return result;
}

template <typename Config>
auto generate(const Board &board, Dice dice) -> std::vector<State<Config>> {
  static_assert(std::is_same_v<Config, WhiteConfig> ||
                std::is_same_v<Config, BlackConfig>);
  auto result = KeepResults<Config>();
  auto states = std::vector<State<Config>>{};
  states.reserve(256);
  states.emplace_back(board, dice);
  uint32_t i = 0;
  while (i < states.size()) {
    State<Config> p = states[i++];
    if (p.dice.size() > 0) {
      auto phase = p.phase();
      if (phase == ON_BAR) {
        for (auto &new_p : moves_on_bar(p)) {
          states.push_back(new_p);
        } 
      } else {
        // In the collecting phase we should consider the largest dice first.
        for (auto d : p.dice.values(phase == COLLECTING)) {
          bool first_move_in_collect = phase == COLLECTING;
          auto bits_copy = p.bits;
          for (auto src_index = bits_copy.begin(); src_index != bits_copy.end(); ++src_index) {
            bool can_collect_wastefully =
                phase == COLLECTING && first_move_in_collect &&
                !Config::is_inboard(Config::apply_dice((*src_index), d));
            bool can_collect_exactly =
                phase == COLLECTING &&
                Config::apply_dice((*src_index), d) == Config::EXACT_COLLECT;
            first_move_in_collect = false;
            if (can_collect_wastefully || can_collect_exactly) {
              auto new_p = p.update_board(*src_index, Config::PLAYER_BAG, d);
              if (new_p.score() != 0) {
                // All winning moves gets the same score, so no need to look
                // further.
                return {new_p};
              }
              states.push_back(new_p);
              break;
            }
            auto dst_index = Config::apply_dice(*src_index, d);
            if (Config::is_inboard(dst_index) &&
                Config::is_valid_dst(p.b[dst_index])) {
              auto new_p = p.update_board(*src_index, dst_index, d);
              states.push_back(new_p);
            }
          }
        }
      }
    }
    result.add(p);
  }
  return result.get_results();
};

} // namespace bg
