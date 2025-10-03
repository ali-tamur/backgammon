#pragma once

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
    auto land_index = Config::LAND_START + Config::SIGN * (d - 1);
    if (Config::is_valid_dst(p.b[land_index])) {
      result.push_back(p.update_board(Config::PLAYER_BAR, land_index, d));
    }
  }
  return result;
}

template <typename Config>
std::vector<State<Config>> move(const Board &board, Dice dice) {
  static_assert(std::is_same_v<Config, WhiteConfig> ||
                std::is_same_v<Config, BlackConfig>);
  auto result = KeepResults<Config>();
  auto states = std::vector<State<Config>>{State<Config>(board, dice)};
  int32_t i = 0;
  while (i < states.size()) {
    auto p = states[i++];
    result.add(p);
    if (p.dice.size() == 0) {
      continue;
    }
    auto phase = p.phase();
    if (phase == ON_BAR) {
      for (auto new_p : moves_on_bar(p)) {
        states.push_back(new_p);
      }
    } else {
      for (auto d : p.dice.values(phase == COLLECTING)) {
        bool first_move_in_collect = phase == COLLECTING;
        for (auto src_index = p.bits.begin(); src_index != p.bits.end();
             ++src_index) {
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
              return {new_p};
            }
            states.push_back(new_p);
            break;
          } else {
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
  }
  auto gg = result.get_results();
  return gg;
};

std::vector<Board> white_move(const Board& board, int32_t d1, int32_t d2) {
  const auto& states = move<WhiteConfig>(board, Dice(d1, d2));
  std::vector<Board> result{};
  std::transform(states.begin(), states.end(), std::back_inserter(result), [](const auto& state) {return state.get_board();});
  return result;
}

std::vector<Board> black_move(const Board& board, int32_t d1, int32_t d2) {
  const auto& states = move<BlackConfig>(board, Dice(d1, d2));
  std::vector<Board> result{};
  std::transform(states.begin(), states.end(), std::back_inserter(result), [](const auto& state) {return state.get_board();});
  return result;
}

}  // namespace bg
