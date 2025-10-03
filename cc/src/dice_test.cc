
#include "dice.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;

namespace bg {

std::vector<int32_t> to_collection(Dice d, bool will_use_max) {
  auto rolls = std::vector<int32_t>{};
  for (auto d : d.values(will_use_max)) {
    rolls.push_back(d);
  }
  return rolls;
}

TEST(DiceTest, Rolls) {
  Dice dice{3, 5};
  EXPECT_EQ(2, dice.size());
  EXPECT_THAT(to_collection(Dice(35), false), ElementsAreArray({5, 3}));
  EXPECT_THAT(to_collection(Dice(35), true), ElementsAreArray({5}));
  EXPECT_THAT(to_collection(Dice(44), false), ElementsAreArray({4}));
  EXPECT_THAT(to_collection(Dice(44), true), ElementsAreArray({4}));
}

TEST(DiceTest, UseDice) {
  Dice dice{3, 5};
  EXPECT_EQ(2, dice.size());

  auto dice_2a = dice.use_dice(3);
  EXPECT_EQ(1, dice_2a.size());
  EXPECT_THAT(to_collection(dice_2a, false), ElementsAreArray({5}));

  auto dice_2b = dice.use_dice(5);
  EXPECT_EQ(1, dice_2b.size());
  EXPECT_THAT(to_collection(dice_2b, false), ElementsAreArray({3}));

  auto dice_3 = dice_2a.use_dice(5);
  EXPECT_EQ(0, dice_3.size());
}

TEST(DiceTest, DoubleRoll) {
  Dice dice{6, 6};
  EXPECT_EQ(4, dice.size());
  EXPECT_THAT(to_collection(dice, false), ElementsAreArray({6}));

  auto d2 = dice.use_dice(6);
  EXPECT_EQ(3, d2.size());
  EXPECT_THAT(to_collection(d2, false), ElementsAreArray({6}));

  auto d3 = d2.use_dice(6);
  EXPECT_EQ(2, d3.size());
  EXPECT_THAT(to_collection(d3, false), ElementsAreArray({6}));

  auto d4 = d3.use_dice(6);
  EXPECT_EQ(1, d4.size());
  EXPECT_THAT(to_collection(d4, false), ElementsAreArray({6}));

  auto d5 = d4.use_dice(6);
  EXPECT_EQ(0, d5.size());
  EXPECT_TRUE(to_collection(d5, false).empty());
}

}  // namespace bg