// Checks generate() against a deliberately simple, slow reference
// implementation of the rules, on positions from random games.
//
// The reference applies the standard rules literally: every order of the
// dice, every stone, all of it brute force. It shares no code with moves.h
// beyond the board layout.

#include <algorithm>
#include <cstdint>
#include <map>
#include <random>
#include <set>
#include <vector>

#include "configs.h"
#include "dice.h"
#include "moves.h"
#include "state.h"
// After the project headers: assertion.h's FAIL must not redefine googletest's.
#include "gtest/gtest.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {
namespace {

constexpr int32_t HOME = 18;  // first point of the mover's home board
constexpr int32_t OFF = 24;   // "point" a borne-off stone moves to
constexpr int32_t BAR = WHITE_BAR;
constexpr int32_t OPP_BAR = BLACK_BAR;
constexpr int32_t BAG = WHITE_BAG;

auto all_home(const Board& b) -> bool {
  if (b[BAR] > 0) {
    return false;
  }
  for (int32_t i = 0; i < HOME; ++i) {
    if (b[i] > 0) {
      return false;
    }
  }
  return true;
}

auto has_won(const Board& b) -> bool { return b[BAG] == NUM_STONES; }

// Moves one stone from `src` (a point, or BAR) by `die`; false if illegal.
auto try_move(const Board& b, int32_t src, int32_t die, Board* out) -> bool {
  if (src == BAR) {
    if (b[BAR] <= 0) {
      return false;
    }
  } else if (b[BAR] > 0 || b[src] <= 0) {
    return false;  // stones on the bar must enter first
  }
  int32_t from = (src == BAR) ? -1 : src;
  int32_t dst = from + die;
  Board r = b;
  r[src] -= 1;
  if (dst >= OFF) {
    if (!all_home(b)) {
      return false;
    }
    if (dst > OFF) {
      // A larger die bears off only the farthest stone from home.
      for (int32_t i = HOME; i < from; ++i) {
        if (b[i] > 0) {
          return false;
        }
      }
    }
    r[BAG] += 1;
    *out = r;
    return true;
  }
  if (b[dst] < -1) {
    return false;  // blocked
  }
  if (b[dst] == -1) {
    r[dst] = 1;
    r[OPP_BAR] -= 1;  // the opponent's bar count is negative
  } else {
    r[dst] += 1;
  }
  *out = r;
  return true;
}

struct Play {
  Board board;
  int32_t used;      // number of dice played
  bool used_larger;  // whether the larger die was played (non-doubles)
};

void explore(const Board& b, std::vector<int32_t> dice, int32_t used, bool used_larger,
             int32_t larger, std::vector<Play>* plays) {
  if (dice.empty() || has_won(b)) {
    // A win ends the turn; it counts as having played every die.
    plays->push_back({b, has_won(b) ? 4 : used, used_larger});
    return;
  }
  bool moved = false;
  std::set<int32_t> tried;
  for (size_t k = 0; k < dice.size(); ++k) {
    int32_t die = dice[k];
    if (!tried.insert(die).second) {
      continue;
    }
    std::vector<int32_t> rest = dice;
    rest.erase(rest.begin() + static_cast<std::ptrdiff_t>(k));
    for (int32_t src = 0; src <= BAR; ++src) {
      if (src >= COLLECT_END && src != BAR) {
        continue;
      }
      Board next{};
      if (try_move(b, src, die, &next)) {
        moved = true;
        explore(next, rest, used + 1, used_larger || die == larger, larger, plays);
      }
    }
  }
  if (!moved) {
    plays->push_back({b, used, used_larger});
  }
}

// All positions the mover may legally reach with this roll.
auto reference_moves(const Board& b, int32_t d1, int32_t d2) -> std::set<Board> {
  std::vector<int32_t> dice =
      (d1 == d2) ? std::vector<int32_t>{d1, d1, d1, d1} : std::vector<int32_t>{d1, d2};
  std::vector<Play> plays;
  explore(b, dice, 0, false, std::max(d1, d2), &plays);
  int32_t most = 0;
  for (const Play& p : plays) {
    most = std::max(most, p.used);
  }
  // If only one die of a non-double can be played, it must be the larger
  // one when that is possible.
  bool larger_rule = false;
  if (d1 != d2 && most == 1) {
    larger_rule =
        std::ranges::any_of(plays, [](const Play& p) { return p.used == 1 && p.used_larger; });
  }
  std::set<Board> result;
  for (const Play& p : plays) {
    if (p.used == most && (!larger_rule || p.used_larger)) {
      result.insert(p.board);
    }
  }
  return result;
}

auto engine_moves(const Board& b, int32_t d1, int32_t d2) -> std::set<Board> {
  std::set<Board> result;
  for (const State& s : generate(b, Dice(d1, d2))) {
    result.insert(s.b);
  }
  return result;
}

// Positions met in random games, from the mover's side, before its roll.
auto random_positions(int32_t n_games, uint32_t seed) -> std::vector<Board> {
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int32_t> die(1, 6);
  std::vector<Board> positions;
  for (int32_t g = 0; g < n_games; ++g) {
    Board b = GAME_START;
    while (true) {
      positions.push_back(b);
      std::vector<State> next = generate(b, Dice(die(rng), die(rng)));
      std::uniform_int_distribution<size_t> pick(0, next.size() - 1);
      b = next[pick(rng)].b;
      if (has_won(b)) {
        break;
      }
      b = flip(b);
    }
  }
  return positions;
}

enum Difference { MISSING, EXTRA };

TEST(MovesReferenceTest, GenerateMatchesTheRules) {
  std::vector<Board> positions = random_positions(300, 2026);
  std::map<std::string, int32_t> counts;
  int32_t checked = 0;
  int32_t shown = 0;
  for (const Board& b : positions) {
    for (int32_t d1 = 1; d1 <= 6; ++d1) {
      for (int32_t d2 = d1; d2 <= 6; ++d2) {
        std::set<Board> expected = reference_moves(b, d1, d2);
        std::set<Board> actual = engine_moves(b, d1, d2);
        ++checked;
        bool wins = std::ranges::any_of(expected, has_won);
        if (wins) {
          // The engine stops at the first winning move: one board, a winning one.
          bool ok = actual.size() == 1 && expected.contains(*actual.begin());
          EXPECT_TRUE(ok) << "winning roll " << d1 << d2;
          continue;
        }
        if (actual != expected) {
          std::vector<Board> missing;
          std::vector<Board> extra;
          std::ranges::set_difference(expected, actual, std::back_inserter(missing));
          std::ranges::set_difference(actual, expected, std::back_inserter(extra));
          std::string kind = (all_home(b) ? std::string("bear-off") : std::string("normal")) +
                             (missing.empty() ? "" : " missing") + (extra.empty() ? "" : " extra");
          ++counts[kind];
          if (shown++ < 5) {
            ADD_FAILURE() << kind << ", roll " << d1 << d2 << ", " << expected.size()
                          << " legal, engine " << actual.size() << "\n board "
                          << ::testing::PrintToString(b);
          }
        }
      }
    }
  }
  for (const auto& [kind, n] : counts) {
    ADD_FAILURE() << n << " of " << checked << " rolls differ: " << kind;
  }
  EXPECT_GT(checked, 100000);
}

// Invariants of every generated move: 15 stones a side, no shared points.
TEST(MovesReferenceTest, StonesAreConserved) {
  for (const Board& b : random_positions(100, 7)) {
    for (int32_t d1 = 1; d1 <= 6; ++d1) {
      for (int32_t d2 = d1; d2 <= 6; ++d2) {
        for (const Board& r : engine_moves(b, d1, d2)) {
          int32_t mine = r[BAR] + r[BAG];
          int32_t theirs = -r[OPP_BAR] - r[BLACK_BAG];
          for (int32_t i = 0; i < COLLECT_END; ++i) {
            mine += std::max<int32_t>(r[i], 0);
            theirs += std::max<int32_t>(-r[i], 0);
          }
          ASSERT_EQ(mine, NUM_STONES);
          ASSERT_EQ(theirs, NUM_STONES);
          ASSERT_GE(r[BAR], 0);
          ASSERT_LE(r[OPP_BAR], 0);
        }
      }
    }
  }
}

}  // namespace
}  // namespace bg
// NOLINTEND(readability-magic-numbers)
