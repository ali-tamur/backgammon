#include "game_manager.h"
#include <iostream>
#include <random>

#include "dice.h"
#include "player.h"
#include "configs.h"
#include "model.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;

namespace bg {

TEST(GameManagerTest, Basic) {
  std::cout << "Test starts \n";

  std::mt19937 gen_{std::random_device{}()};
  auto alice = Player<RANDOM, WhiteConfig>("alice", gen_);
  auto bob = Player<RANDOM, BlackConfig>("bob", gen_);
  auto dice_throw = DiceThrow();
  GameOptions options{};
  options.white_name = alice.name;
  options.black_name = bob.name;
  options.white_model = "";
  options.black_model = "";
  options.n_games = 1;
  options.is_interactive = false;
  options.print_score = true;
  std::stringstream ss;
  options.outstream = &ss;
  auto game_manager = game_manager_factory(options);
  auto [white_score, black_score] = game_manager.run();
  std::cout << "White: " << white_score << " Black: " << black_score;
  EXPECT_EQ(white_score >= 0 && white_score <= 3 && black_score >= 0 &&
                black_score <= 3 && white_score * black_score == 0 &&
                white_score + black_score > 0,
            true);

  LogReader reader(ss);
  auto games = reader.read_all();
  EXPECT_EQ(games.size(), 1);
};

} // namespace bg