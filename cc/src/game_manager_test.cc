#include "game_manager.h"
#include <random>
#include <iostream>

#include "player.h"
#include "dice.h"
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
  options.is_interactive = false;
  options.will_log = false;
  auto white_move =  [&alice](const Board& b, Dice d) -> auto {return alice.move(b, d);};
  auto black_move =  [&bob](const Board& b, Dice d) -> auto {return bob.move(b, d);};
  auto game_manager = GameManager(white_move, black_move, dice_throw, options);
  auto [white_score, black_score] = game_manager.run(1);
  std::cout << "White: " << white_score << " Black: " << black_score;
  EXPECT_EQ(
    white_score >= 0 && 
    white_score <= 3 && 
    black_score >= 0 && 
    black_score <= 3 && 
    white_score*black_score == 0 && 
    white_score+black_score > 0, true);
    EXPECT_EQ(5,5);
};


}  // namespace bg