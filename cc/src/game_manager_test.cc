#include "game_manager.h"

#include <iostream>
#include <random>

#include "configs.h"
#include "dice.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "model.h"
#include "player.h"

using namespace ::testing;

namespace bg {

TEST(GameManagerTest, Basic) {
  std::mt19937 gen_{std::random_device{}()};
  auto dice_throw = DiceThrow();
  GameOptions options{};
  options.white_model = "random";
  options.black_model = "random";
  options.n_games = 1;
  options.is_interactive = false;
  options.print_score = true;
  std::stringstream ss;
  options.outstream = &ss;
  {
    auto game_manager = game_manager_factory(options);
    auto [white_score, black_score] = game_manager.run();
    EXPECT_EQ(white_score >= 0 && white_score <= 3 && black_score >= 0 && black_score <= 3 &&
                  white_score * black_score == 0 && white_score + black_score > 0,
              true);
  }
  LogReader reader(ss);
  auto games = reader.read_all();
  EXPECT_EQ(games.size(), 1);
};

}  // namespace bg