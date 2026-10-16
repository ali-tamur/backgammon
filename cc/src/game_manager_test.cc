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

// Game ids are timestamps; everything else must match.
auto same_game(const Game& a, const Game& b) -> bool {
  if (a.final_score() != b.final_score() || a.player() != b.player() ||
      a.positions_size() != b.positions_size() || a.dice_size() != b.dice_size()) {
    return false;
  }
  for (int32_t i = 0; i < a.positions_size(); ++i) {
    if (a.positions(i).SerializeAsString() != b.positions(i).SerializeAsString()) {
      return false;
    }
  }
  for (int32_t i = 0; i < a.dice_size(); ++i) {
    if (a.dice(i) != b.dice(i)) {
      return false;
    }
  }
  return true;
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
  EXPECT_EQ(games[0].player(), FirstPlayer::WHITE);
  EXPECT_EQ(games[1].player(), FirstPlayer::BLACK);
  EXPECT_EQ(games[2].player(), FirstPlayer::WHITE);
  EXPECT_EQ(games[3].player(), FirstPlayer::BLACK);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace bg