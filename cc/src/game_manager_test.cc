#include "game_manager.h"

#include <iostream>
#include <random>

#include "configs.h"
#include "dice.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "model.h"
#include "player.h"

namespace bg {

TEST(GameManagerTest, Basic) {
  std::string temp_path = std::tmpnam(nullptr);
  GameOptions options{};
  options.white_model = "random";
  options.black_model = "random";
  options.n_games = 1;
  options.is_interactive = false;
  options.print_score = true;
  options.log_path = temp_path;
  options.is_unittest = true;
  {
    auto game_manager = game_manager_factory(options);
    auto [white_score, black_score] = game_manager.run();
    EXPECT_EQ(white_score >= 0 && white_score <= 3 && black_score >= 0 && black_score <= 3 &&
                  white_score * black_score == 0 && white_score + black_score > 0,
              true);
  }
  std::ifstream file(temp_path, std::ios::binary);
  LogReader reader(file);
  auto games = reader.read_all();
  EXPECT_EQ(games.size(), 1);
};

// NOLINTBEGIN(readability-magic-numbers)

// Black's last stone is on point 2 (black bears off past index 0).
auto black_about_to_win() -> Board {
  Board b{};
  b[2] = -1;
  b[BLACK_BAG] = -14;
  b[10] = 15;
  return b;
}

TEST(PlayTurnTest, BlackWinIsNegative) {
  auto player = construct_random_player_move(std::make_shared<std::mt19937>(1));
  auto [after, result] = play_turn(player, black_about_to_win(), Dice(6, 5), false);
  EXPECT_EQ(result, -2);  // white has borne off nothing: gammon
  EXPECT_EQ(after[2], 0);
  EXPECT_EQ(after[BLACK_BAG], -15);
  EXPECT_EQ(after[10], 15);
}

TEST(PlayTurnTest, BlackBackgammon) {
  Board b = black_about_to_win();
  b[10] = 14;
  b[3] = 1;  // a white stone in black's home
  auto player = construct_random_player_move(std::make_shared<std::mt19937>(1));
  auto [after, result] = play_turn(player, b, Dice(6, 5), false);
  EXPECT_EQ(result, -3);
}

TEST(PlayTurnTest, WhiteWinIsPositive) {
  auto player = construct_random_player_move(std::make_shared<std::mt19937>(1));
  auto [after, result] = play_turn(player, flip(black_about_to_win()), Dice(6, 5), true);
  EXPECT_EQ(result, 2);
  EXPECT_EQ(after, flip(Board{[] {
              Board b = black_about_to_win();
              b[2] = 0;
              b[BLACK_BAG] = -15;
              return b;
            }()}));
}

TEST(PlayTurnTest, BlackOpeningMoveLeavesWhiteAlone) {
  auto player = construct_random_player_move(std::make_shared<std::mt19937>(1));
  auto [after, result] = play_turn(player, GAME_START, Dice(3, 1), false);
  EXPECT_EQ(result, 0);
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    if (GAME_START[i] > 0) {
      EXPECT_EQ(after[i], GAME_START[i]) << "white stones moved at " << i;
    }
  }
  EXPECT_NE(after, GAME_START);
}

// Plays a short match into a temporary log and returns the games.
auto play_logged_match(uint64_t seed, int32_t n_games) -> std::vector<Game> {
  std::string path = std::tmpnam(nullptr);
  GameOptions options{};
  options.white_model = "random";
  options.black_model = "random";
  options.n_games = n_games;
  options.first_move = FirstMove::Alternate;
  options.log_path = path;
  options.is_unittest = true;
  options.seed = seed;
  {
    auto game_manager = game_manager_factory(options);
    game_manager.run();
  }
  std::ifstream file(path, std::ios::binary);
  return LogReader(file).read_all();
}

auto same_game(const Game& a, const Game& b) -> bool {
  return a.SerializeAsString() == b.SerializeAsString();
}

TEST(SeedTest, SameSeedSameGames) {
  auto first = play_logged_match(42, 10);
  auto second = play_logged_match(42, 10);
  ASSERT_EQ(first.size(), 10);
  ASSERT_EQ(second.size(), 10);
  for (size_t i = 0; i < first.size(); ++i) {
    EXPECT_TRUE(same_game(first[i], second[i])) << "game " << i;
  }
}

TEST(SeedTest, DifferentSeedsDifferentGames) {
  auto first = play_logged_match(42, 3);
  auto second = play_logged_match(43, 3);
  EXPECT_FALSE(same_game(first[0], second[0]));
}

TEST(SeedTest, GamesDifferWithinAMatch) {
  auto games = play_logged_match(42, 2);
  EXPECT_FALSE(same_game(games[0], games[1]));
}

TEST(SeedTest, FirstPlayerAlternates) {
  auto games = play_logged_match(42, 4);
  EXPECT_TRUE(games[0].white_first());
  EXPECT_FALSE(games[1].white_first());
  EXPECT_TRUE(games[2].white_first());
  EXPECT_FALSE(games[3].white_first());
}

// Checks the record against game.proto's description of it.
TEST(LogTest, GameRecordShape) {
  auto cells = [](const Position& p) {
    return std::vector<int32_t>(p.cell().begin(), p.cell().end());
  };
  for (const Game& game : play_logged_match(7, 20)) {
    EXPECT_EQ(cells(game.start()), std::vector<int32_t>(GAME_START.begin(), GAME_START.end()));
    ASSERT_GT(game.turns_size(), 0);
    for (const Turn& turn : game.turns()) {
      int32_t high = turn.dice() / 10;
      int32_t low = turn.dice() % 10;
      EXPECT_TRUE(1 <= low && low <= high && high <= 6) << turn.dice();
      EXPECT_EQ(turn.after().cell_size(), BOARD_SIZE);
      EXPECT_FALSE(turn.has_prediction());  // random players
    }
    // The first move is made by the colour the record says moved first.
    auto after_first = cells(game.turns(0).after());
    bool white_moved = false;
    for (int32_t i = 0; i < COLLECT_END; ++i) {
      if (GAME_START[i] > 0 && after_first[i] < GAME_START[i]) {
        white_moved = true;
      }
    }
    EXPECT_EQ(white_moved, game.white_first());
    // The final position shows the winner with all 15 stones borne off.
    auto last = cells(game.turns(game.turns_size() - 1).after());
    EXPECT_EQ(game.final_score() > 0 ? last[WHITE_BAG] : -last[BLACK_BAG], NUM_STONES);
  }
}

// NOLINTEND(readability-magic-numbers)

}  // namespace bg