
#include "state.h"

#include <utility>

#include "configs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "moves.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {

struct Change {
  struct ValueTag {};
  struct DeltaTag {};

  Change(int32_t index, int32_t value, ValueTag tag)
      : index{index}, value{value}, delta{0}, is_absolute{true} {}
  Change(int32_t index, int32_t value, DeltaTag tag)
      : index{index}, value{0}, delta{value}, is_absolute{false} {}

  int32_t index;
  int32_t value;
  int32_t delta;
  bool is_absolute;
};

constexpr Change::ValueTag VALUE{};
constexpr Change::DeltaTag DELTA{};

struct Case {
  Case(std::set<Board> result, int32_t size, std::set<Board> expectation)
      : result(std::move(result)), expectation(std::move(expectation)), size(size) {}
  std::set<Board> result;
  std::set<Board> expectation;
  int32_t size;
};

class Expectation {
 public:
  Expectation(Board board, Dice dice) : b{board}, dice(dice) {}

  void add_expectation(const std::vector<Change>& changes) {
    Board e = b;
    for (const auto& change : changes) {
      if (change.is_absolute) {
        e[change.index] = static_cast<int8_t>(change.value);
      } else {
        e[change.index] = static_cast<int8_t>(e[change.index] + change.delta);
      }
    }
    expectations.push_back(e);
  }

  auto get_test_result() -> Case {
    std::set<Board> expectation_set(expectations.begin(), expectations.end());
    std::vector<State> result = generate(b, dice);
    int32_t result_size = result.size();
    std::set<Board> result_set{};
    for (const auto& r : result) {
      result_set.insert(r.b);
    }
    return {result_set, result_size, expectation_set};
  }

 private:
  Board b;
  Dice dice;
  std::vector<Board> expectations;
};

auto generate_all_dice() -> std::vector<Dice> {
  auto result = std::vector<Dice>{};
  for (int32_t d1 = 1; d1 <= 6; ++d1) {
    for (int32_t d2 = d1; d2 <= 6; ++d2) {
      result.emplace_back(d1, d2);
    }
  }
  return result;
}

// Expects exactly one resulting board, equal to the expectation.
void expect_single_result(Expectation& e) {
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, StoneOnBarCannotLand) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .  *3   .   .   .   . ││ *2  *2  *2  *2  *2  *2 │
  //  │                        ││                        │
  //  │                        ││                        │   1 :
  //  │  .   .   .   .   .   . ││ 14   .   .   .   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, -2, -2, -2, -2, 0, 0, 0, 0, -3, 0, 0, 0,
                                  0,  0,  0,  0,  14, 0,  0, 0, 0, 0, 1,  0, 0, 0};
  for (auto dice : generate_all_dice()) {
    Expectation e(b, dice);
    e.add_expectation({});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TEST(StateTest, StoneOnBarCannotLand2) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  . *11   .   .   .   . ││  .   .  *2   .  *2   . │
  //  │                        ││                        │
  //  │                        ││                        │   1 :
  //  │  .   .   .   .   .   . ││ 14   .   .   .   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  // Indexes: [0:23, W-bolted, B-bolted, W-collect, B-collect]
  // W numbers always positive, B numbers always negative.
  auto b = std::array<int8_t, 28>{0, -2, 0, -2, 0,  0, 0, 0, 0, 0, -11, 0, 0, 0,
                                  0, 0,  0, 0,  14, 0, 0, 0, 0, 0, 1,   0, 0, 0};
  for (const auto& [d1, d2] : {std::make_pair(2, 2), {2, 4}, {4, 4}}) {
    Expectation e(b, Dice(d1, d2));
    e.add_expectation({});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TEST(StateTest, StoneCannotMove) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .  *3   .   .   .  *2 ││ *2  *2  *2  *2  *2   2 │
  //  │                        ││                        │
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  // Indexes: [0:23, W-bolted, B-bolted, W-collect, B-collect]
  // W numbers always positive, B numbers always negative.
  auto b = std::array<int8_t, 28>{2, -2, -2, -2, -2, -2, -2, 0, 0, 0,  -3, 0, 0, 0,
                                  0, 0,  0,  0,  0,  0,  0,  0, 0, 13, 0,  0, 0, 0};

  for (auto dice : generate_all_dice()) {
    Expectation e(b, dice);
    e.add_expectation({});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TEST(StateTest, StoneCannotMove2) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   . *15 ││  .   .   .   .   .   3 │
  //  │                        ││                        │
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  2   2   2   2   2   2 │
  //  └────────────────────────┴┴────────────────────────┘
  // Indexes: [0:23, W-bolted, B-bolted, W-collect, B-collect]
  // W numbers always positive, B numbers always negative.
  auto b = std::array<int8_t, 28>{3, 0, 0, 0, 0, 0, -15, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 2, 2, 2,   2, 2, 2, 0, 0, 0, 0};
  Expectation e(b, Dice(6, 6));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, RemainingStoneCanNeitherMoveNorGetCollected) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │       : *13
  //  │                        ││                        │       :  13
  //  │  .   .   .   .   .   . ││  2   .   .   .   .  *2 │
  //  └────────────────────────┴┴────────────────────────┘
  // Indexes: [0:23, W-bolted, B-bolted, W-collect, B-collect]
  // W numbers always positive, B numbers always negative.
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,   0,  0,
                                  0, 0, 0, 0, 2, 0, 0, 0, 0, -2, 0, -13, 13, 0};
  Expectation e(b, Dice(5, 5));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, NoStoneCanMove) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   2   .  *2  *2   . ││  2   .   .   .   .   . │
  //  │                        ││                        │       :  *7
  //  │                        ││                        │       :  11
  //  │ *2  *2   .   .   .   . ││  .   .   .   .   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  // Indexes: [0:23, W-bolted, B-bolted, W-collect, B-collect]
  // W numbers always positive, B numbers always negative.
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 2, 0, -2, -2, 0, 2, 0, -2, -2,
                                  0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 11, -7};
  Expectation e(b, Dice(2, 3));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, NoStoneCanMove2) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │      : *11
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   2   .   .  -2  -2 │      : 13
  //  └────────────────────────┴┴────────────────────────┘
  // Indexes: [0:23, W-bolted, B-bolted, W-collect, B-collect]
  // W numbers always positive, B numbers always negative.
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0,  0,
                                  0, 0, 0, 0, 0, 2, 0, 0, -2, -2, 0, 0, 13, 11};
  Expectation e(b, Dice(4, 3));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, LandOneStone) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││ *2  *2  *2   .  *2  *2 │
  //  │                        ││                        │     :  *5
  //  │                        ││                        │   2 :
  //  │  .   .   .   .   .   . ││  .  13   .   .   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, 0, -2, -2, -2, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0,  0,  0, 0,  0,  13, 0, 0, 0, 0, 2, 0, 0, -5};
  Expectation e(b, Dice(4, 3));
  e.add_expectation({{2, 1, VALUE}, {WHITE_BAR, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, LandTwoStones) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││ *2  *2  *2   .  *2  *2 │
  //  │                        ││                        │       :  *5
  //  │                        ││                        │   2 :
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, 0, -2, -2, -2, 0, 0, 0, 0,  0, 0, 0, 0,
                                  0,  0,  0, 0,  0,  0,  0, 0, 0, 13, 2, 0, 0, -5};
  Expectation e(b, Dice(3, 3));
  e.add_expectation({{2, 2, VALUE}, {WHITE_BAR, 0, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, Escape1) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .  *2  *2  *2  *2  *2 │
  //  │                        ││                        │       :  *5
  //  │                        ││                        │   1 :
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  14 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, -2, -2, -2, 0, 0, 0, 0, 0,  0, 0, 0, 0,
                                  0,  0,  0,  0,  0,  0, 0, 0, 0, 14, 1, 0, 0, -5};
  Expectation e(b, Dice(6, 6));
  e.add_expectation({{WHITE_BAR, 0, VALUE}, {COLLECT_END - 1, 15, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, Escape2HaveToUseSmallerDiceFirst) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││ *2   .   .   .   .   1 │
  //  │                        ││                        │ *13 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  14 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0, 0, 0, 0, -2, 0, 0, 0, 0,  0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0,  0, 0, 0, 14, 0, 0, 0, -13};
  Expectation e(b, Dice(5, 2));
  e.add_expectation({{0, 0, VALUE}, {7, 1, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, SingleStoneMovement) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  14 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, -15, 0, 0};
  for (int32_t i = 1; i <= 6; ++i) {
    for (int32_t j = i + 1; j <= 6; ++j) {
      Expectation e(b, Dice(i, j));
      e.add_expectation({{0, 0, VALUE}, {i + j, 1, DELTA}});
      expect_single_result(e);
    }
  }

  for (int32_t i = 1; i <= 5; ++i) {
    Expectation e(b, Dice(i, i));
    e.add_expectation({{0, 0, VALUE}, {4 * i, 1, DELTA}});
    expect_single_result(e);
  }

  Expectation e(b, Dice(6, 6));
  e.add_expectation({{0, 0, VALUE}, {WHITE_BAG, 1, VALUE}});
  expect_single_result(e);
}

TEST(StateTest, SingleStoneMoveAndBolt) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .  *1   . ││  .   .   .   .   .   1 │
  //  │                        ││                        │ *14 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  14 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0, 0, 0, 0, 0, 0, -1, 0, 0,  0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0,  0, 14, 0, 0, 0, -14};
  for (int32_t i = 1; i <= 6; ++i) {
    int32_t j = 7 - i;
    Expectation e(b, Dice(i, j));
    e.add_expectation({{0, 0, VALUE}, {7, 1, VALUE}, {BLACK_BAR, -1, VALUE}});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TEST(StateTest, DoubleStoneMovement) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   1   .   .   .   . ││  .   .   .   .   .   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, -15, 0, 0};
  for (int32_t i = 1; i <= 6; ++i) {
    for (int32_t j = i + 1; j <= 6; ++j) {
      Expectation e(b, Dice(i, j));
      for (int32_t src : {0, 10}) {
        e.add_expectation({{src, 0, VALUE}, {src + i + j, 1, DELTA}});
      }
      for (int32_t src1 : {0, 10}) {
        auto src2 = 10 - src1;
        e.add_expectation(
            {{src1, 0, VALUE}, {src2, 0, VALUE}, {src1 + i, 1, VALUE}, {src2 + j, 1, VALUE}});
      }
      auto r = e.get_test_result();
      EXPECT_EQ(r.size, 4);
      EXPECT_EQ(r.result, r.expectation);
    }
  }
}

TEST(StateTest, DoubleStoneMovementWithBlocks) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .  *2  *2  *2 ││  .   .   .  *2   1   1 │
  //  │                        ││                        │  *7 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 1, -2, 0, 0, 0, -2, -2, -2, 0,  0, 0,  0, 0,
                                  0, 0, 0,  0, 0, 0, 0,  0,  0,  13, 0, -7, 0, 0};
  Expectation e(b, Dice(5, 2));
  e.add_expectation({{0, 0, VALUE}, {1, 0, VALUE}, {3, 1, VALUE}, {5, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, DoubleStoneMovementWithBlocks2) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .  *2   .  *2 ││ *2   .   .   .   1   1 │
  //  │                        ││                        │  *9 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘

  auto b = std::array<int8_t, 28>{1, 1, 0, 0, 0, -2, -2, 0, -2, 0,  0, 0,  0, 0,
                                  0, 0, 0, 0, 0, 0,  0,  0, 0,  13, 0, -9, 0, 0};
  Expectation e(b, Dice(5, 2));
  e.add_expectation({{0, 0, VALUE}, {7, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, OnlyOneDiceCanMoveMultipleResults) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │ *2  *2   .   .   .  *2 ││  .   1   .   .   .   1 │
  //  │                        ││                        │  *5 :
  //  │                        ││                        │
  //  │  .   .   .  *2   .   . ││ 13   .   .   .   .  *2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0,  0, 0, 1,  0, -2, 0, 0, 0,  -2, -2, 0, 0,
                                  0, -2, 0, 0, 13, 0, 0,  0, 0, -2, 0,  -5, 0, 0};
  Expectation e(b, Dice(5, 6));
  e.add_expectation({{0, 0, VALUE}, {5, 1, VALUE}});
  e.add_expectation({{4, 0, VALUE}, {9, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 2);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, DoubleStoneDoubleDice) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   1   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, -15, 0, 0};
  Expectation e(b, Dice(3, 3));
  for (int32_t move_first = 0; move_first < 5; ++move_first) {
    int32_t move_second = 4 - move_first;
    e.add_expectation({{0, 0, VALUE},
                       {move_first * 3, 1, VALUE},
                       {1, 0, VALUE},
                       {1 + (move_second * 3), 1, VALUE}});
  }
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 5);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, DoubleStoneDoubleDice2) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   1   .   .   .   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0, 0, 0, 1, 0, 0, 0, 0, 0,  0, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, -15, 0, 0};
  Expectation e(b, Dice(4, 4));

  for (int32_t move_first = 0; move_first < 5; ++move_first) {
    auto move_second = 4 - move_first;
    e.add_expectation({{4, 0, VALUE},
                       {4 + (move_second * 4), 1, DELTA},
                       {0, 0, VALUE},
                       {move_first * 4, 1, DELTA}});
  }
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 3);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, FourStonesDoubleDice) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   1   1   1   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  11 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  0, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, -15, 0, 0};
  Expectation e(b, Dice(4, 4));
  for (int32_t move_a = 0; move_a < 5; ++move_a) {
    for (int32_t move_b = 0; move_b < 5 - move_a; ++move_b) {
      for (int32_t move_c = 0; move_c < 5 - move_a - move_b; ++move_c) {
        int32_t move_d = 4 - move_a - move_b - move_c;

        e.add_expectation({{0, 0, VALUE},
                           {1, 0, VALUE},
                           {2, 0, VALUE},
                           {3, 0, VALUE},
                           {move_a * 4, 1, DELTA},
                           {1 + (move_b * 4), 1, DELTA},
                           {2 + (move_c * 4), 1, DELTA},
                           {3 + (move_d * 4), 1, DELTA}});
      }
    }
  }
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 35);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, CollectWithWaste) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │     :  12
  //  │  .   .   .   .   .   . ││  .   .   .   1   1   1 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, -15, 12, 0};
  Expectation e(b, Dice(4, 5));
  e.add_expectation(
      {{WHITE_BAG, 14, VALUE}, {COLLECT_END - 3, 0, VALUE}, {COLLECT_END - 2, 0, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, CollectDouble) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││ 10   1   1   1   1   1 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0,   0, 0,
                                  0, 0, 0, 0, 10, 1, 1, 1, 1, 1, 0, -15, 0, 0};
  Expectation e(b, Dice(6, 6));
  e.add_expectation({{COLLECT_END - 6, 6, VALUE}, {WHITE_BAG, 4, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, MoveInCollectPhase) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │     :  13
  //  │  .   .   .   .   .   . ││  1   .   .   .   .   1 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -15, 13, 0};
  Expectation e(b, Dice(3, 2));
  e.add_expectation({{COLLECT_END - 6, 0, VALUE}, {COLLECT_END - 1, 2, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, PartialCollect) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *13 :
  //  │                        ││                        │     :  11
  //  │  .   .   .   .   .   . ││  .   1   .   1  *2   2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 1, 0, 1, -2, 2, 0, -13, 11, 0};
  Expectation e(b, Dice(3, 3));
  e.add_expectation({{COLLECT_END - 3, 0, VALUE}, {WHITE_BAG, 1, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, MoveAndCollect) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │     :  13
  //  │  .   .   .   .   .   . ││  1   .   .   1   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, -15, 13, 0};
  Expectation e(b, Dice(5, 3));
  e.add_expectation({{COLLECT_END - 6, 0, VALUE},
                     {COLLECT_END - 1, 1, VALUE},
                     {COLLECT_END - 3, 0, VALUE},
                     {WHITE_BAG, 1, DELTA}});
  // The 3 first: 18->21, then the 5 bears off from 21, now the farthest stone.
  e.add_expectation({{COLLECT_END - 6, 0, VALUE}, {WHITE_BAG, 1, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 2);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, MoveAllNoCollect) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *13 :
  //  │                        ││                        │     :  10
  //  │  .   .   .   .   .   . ││  .   .   .   5   .  *2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 0, 0, 5, 0, -2, 0, -13, 10, 0};
  Expectation e(b, Dice(1, 1));
  e.add_expectation({{COLLECT_END - 3, -4, DELTA}, {COLLECT_END - 2, 4, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TEST(StateTest, GameEnd) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │     :  12
  //  │  .   .   .   .   .   . ││  .   1   .   1   .   1 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, -15, 12, 0};
  std::vector<Dice> dices{Dice(6, 6), Dice(5, 5), Dice(4, 4), Dice(3, 3)};
  for (const auto& dice : dices) {
    Expectation e(b, dice);
    e.add_expectation({{COLLECT_END - 1, 0, VALUE},
                       {COLLECT_END - 3, 0, VALUE},
                       {COLLECT_END - 5, 0, VALUE},
                       {WHITE_BAG, 15, VALUE}});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TEST(StateTest, CountMovesAtStart) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  5   .   .   .  *3   . ││ *5   .   .   .   .   2 │
  //  │                        ││                        │
  //  │                        ││                        │
  //  │ *5   .   .   .   3   . ││  5   .   .   .   .  *2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{2, 0, 0, 0, 0, -5, 0, -3, 0, 0,  0, 5, -5, 0,
                                  0, 0, 3, 0, 5, 0,  0, 0,  0, -2, 0, 0, 0,  0};

  {
    Expectation e(b, Dice(6, 6));
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 11);
  }

  {
    Expectation e(b, Dice(5, 5));
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 4);
  }

  {
    Expectation e(b, Dice(1, 5));
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 8);
  }
}

// Only one die can be played (11->14 or 11->16, then stuck): it must be the
// larger, hitting the blot on 16.
TEST(StateTest, OnlyOneDieCanBePlayedMustBeTheLarger) {
  auto b = std::array<int8_t, 28>{-4, 0, 0,  -2, 0, 0,  0, -2, 0, 0, 0, 1, -1, 0,
                                  0,  0, -1, -1, 0, -4, 0, 1,  7, 6, 0, 0, 0,  0};
  Expectation e(b, Dice(3, 5));
  e.add_expectation({{11, 0, VALUE}, {16, 1, VALUE}, {BLACK_BAR, -1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

// The opponent holds 23. Bearing off from 22 with the 2 must still leave the 1
// for 21->22; the generator used to drop it and play a single die.
TEST(StateTest, BearOffUsesBothDiceWhenPossible) {
  auto b = std::array<int8_t, 28>{-5, -3, 0,  0, -1, 0, 0, -1, 0,  0,  0, -1, 0, 0,
                                  0,  0,  -1, 0, -1, 0, 0, 3,  10, -2, 0, 0,  2, 0};
  Expectation e(b, Dice(1, 2));
  e.add_expectation({{21, 2, VALUE}, {WHITE_BAG, 3, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

// Playing the smaller die first can give positions the larger-first order
// cannot: here 20->21 then the 5 bears off from 21, leaving 22.
TEST(StateTest, BearOffCanPlayTheSmallerDieFirst) {
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, -15, 13, 0};
  Expectation e(b, Dice(1, 5));
  e.add_expectation({{20, 0, VALUE}, {22, 0, VALUE}, {23, 1, VALUE}, {WHITE_BAG, 14, VALUE}});
  e.add_expectation({{20, 0, VALUE}, {WHITE_BAG, 14, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 2);
  EXPECT_EQ(r.result, r.expectation);
}

}  // namespace bg
// NOLINTEND(readability-magic-numbers)
