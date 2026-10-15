
#include "state.h"

#include <utility>

#include "configs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "moves.h"

using namespace ::testing;

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

template <typename Config>
class Expectation {
 public:
  Expectation(Board board, Dice dice) : b{board}, dice(dice) {}

  void add_expectation(const std::vector<Change>& changes) {
    Board e = b;
    for (const auto& change : changes) {
      if (change.is_absolute) {
        e[change.index] = change.value;
      } else {
        e[change.index] += change.delta;
      }
    }
    expectations.push_back(e);
  }

  auto get_test_result() -> Case {
    Board final_b;
    if (Config::COLOR == "black") {
      final_b = opponent_perspective(b);
      for (auto& expectation : expectations) {
        expectation = opponent_perspective(expectation);
      }
    } else {
      final_b = b;
    }
    std::set<Board> expectation_set(expectations.begin(), expectations.end());
    std::vector<State<Config>> result = generate<Config>(final_b, dice);
    int32_t result_size = result.size();
    std::set<Board> result_set{};
    for (const auto& r : result) {
      result_set.insert(r.b);
    }
    return {result_set, result_size, expectation_set};
  }

 private:
  auto opponent_perspective(const Board& b) -> Board {
    Board r = {};
    for (int32_t i = 0; i < COLLECT_END; ++i) {
      r[i] = -b[COLLECT_END - 1 - i];
    }
    r[WHITE_BAR] = -b[BLACK_BAR];
    r[BLACK_BAR] = -b[WHITE_BAR];
    r[WHITE_BAG] = -b[BLACK_BAG];
    r[BLACK_BAG] = -b[WHITE_BAG];
    return r;
  }
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

template <typename T>
class StateTest : public ::testing::Test {
  using Config = T;
};

using PlayerConfigTypes = ::testing::Types<WhiteConfig, BlackConfig>;
TYPED_TEST_SUITE(StateTest, PlayerConfigTypes);

TYPED_TEST(StateTest, anything) {
  const std::string_view color = TypeParam::COLOR;
  EXPECT_TRUE(color == "white" || color == "black");
}

TYPED_TEST(StateTest, stone_on_bar_cannot_land) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .  *3   .   .   .   . ││ *2  *2  *2  *2  *2  *2 │
  //  │                        ││                        │
  //  │                        ││                        │   1 :
  //  │  .   .   .   .   .   . ││ 14   .   .   .   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, -2, -2, -2, -2, 0, 0, 0, 0, -3, 0, 0, 0,
                                  0,  0,  0,  0,  14, 0,  0, 0, 0, 0, 1,  0, 0, 0};
  for (auto dice : generate_all_dice()) {
    Expectation<TypeParam> e(b, dice);
    e.add_expectation({});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TYPED_TEST(StateTest, stone_on_bar_cannot_land_2) {
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
    Expectation<TypeParam> e(b, Dice(d1, d2));
    e.add_expectation({});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TYPED_TEST(StateTest, stone_cannot_move) {
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
    Expectation<TypeParam> e(b, dice);
    e.add_expectation({});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TYPED_TEST(StateTest, stone_cannot_move_2) {
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
  Expectation<TypeParam> e(b, Dice(6, 6));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, remaining_stone_can_neither_move_nor__get_collected) {
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
  Expectation<TypeParam> e(b, Dice(5, 5));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, no_stone_can_move) {
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
  Expectation<TypeParam> e(b, Dice(2, 3));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, no_stone_can_move_2) {
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
  Expectation<TypeParam> e(b, Dice(4, 3));
  e.add_expectation({});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, land_one_stone) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││ *2  *2  *2   .  *2  *2 │
  //  │                        ││                        │     :  *5
  //  │                        ││                        │   2 :
  //  │  .   .   .   .   .   . ││  .  13   .   .   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, 0, -2, -2, -2, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0,  0,  0, 0,  0,  13, 0, 0, 0, 0, 2, 0, 0, -5};
  Expectation<TypeParam> e(b, Dice(4, 3));
  e.add_expectation({{2, 1, VALUE}, {WHITE_BAR, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, land_two_stones) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││ *2  *2  *2   .  *2  *2 │
  //  │                        ││                        │       :  *5
  //  │                        ││                        │   2 :
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, 0, -2, -2, -2, 0, 0, 0, 0,  0, 0, 0, 0,
                                  0,  0,  0, 0,  0,  0,  0, 0, 0, 13, 2, 0, 0, -5};
  Expectation<TypeParam> e(b, Dice(3, 3));
  e.add_expectation({{2, 2, VALUE}, {WHITE_BAR, 0, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, escape_1) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .  *2  *2  *2  *2  *2 │
  //  │                        ││                        │       :  *5
  //  │                        ││                        │   1 :
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  14 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{-2, -2, -2, -2, -2, 0, 0, 0, 0, 0,  0, 0, 0, 0,
                                  0,  0,  0,  0,  0,  0, 0, 0, 0, 14, 1, 0, 0, -5};
  Expectation<TypeParam> e(b, Dice(6, 6));
  e.add_expectation({{WHITE_BAR, 0, VALUE}, {COLLECT_END - 1, 15, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, escape_2_have_to_use_smaller_dice_first) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││ *2   .   .   .   .   1 │
  //  │                        ││                        │ *13 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  14 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0, 0, 0, 0, -2, 0, 0, 0, 0,  0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0,  0, 0, 0, 14, 0, 0, 0, -13};
  Expectation<TypeParam> e(b, Dice(5, 2));
  e.add_expectation({{0, 0, VALUE}, {7, 1, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, single_stone_movement) {
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
      Expectation<TypeParam> e(b, Dice(i, j));
      e.add_expectation({{0, 0, VALUE}, {i + j, 1, DELTA}});
      auto r = e.get_test_result();
      EXPECT_EQ(r.size, 1);
      EXPECT_EQ(r.result, r.expectation);
    }
  }

  for (int32_t i = 1; i <= 5; ++i) {
    Expectation<TypeParam> e(b, Dice(i, i));
    e.add_expectation({{0, 0, VALUE}, {4 * i, 1, DELTA}});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }

  Expectation<TypeParam> e(b, Dice(6, 6));
  e.add_expectation({{0, 0, VALUE}, {WHITE_BAG, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, single_stone_move_and_bolt) {
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
    Expectation<TypeParam> e(b, Dice(i, j));
    e.add_expectation({{0, 0, VALUE}, {7, 1, VALUE}, {BLACK_BAR, -1, VALUE}});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TYPED_TEST(StateTest, double_stone_movement) {
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
      Expectation<TypeParam> e(b, Dice(i, j));
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

TYPED_TEST(StateTest, double_stone_movement_with_blocks) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .  *2  *2  *2 ││  .   .   .  *2   1   1 │
  //  │                        ││                        │  *7 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 1, -2, 0, 0, 0, -2, -2, -2, 0,  0, 0,  0, 0,
                                  0, 0, 0,  0, 0, 0, 0,  0,  0,  13, 0, -7, 0, 0};
  Expectation<TypeParam> e(b, Dice(5, 2));
  e.add_expectation({{0, 0, VALUE}, {1, 0, VALUE}, {3, 1, VALUE}, {5, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, double_stone_movement_with_blocks_2) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .  *2   .  *2 ││ *2   .   .   .   1   1 │
  //  │                        ││                        │  *9 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘

  auto b = std::array<int8_t, 28>{1, 1, 0, 0, 0, -2, -2, 0, -2, 0,  0, 0,  0, 0,
                                  0, 0, 0, 0, 0, 0,  0,  0, 0,  13, 0, -9, 0, 0};
  Expectation<TypeParam> e(b, Dice(5, 2));
  e.add_expectation({{0, 0, VALUE}, {7, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, only_one_dice_can_move_multiple_results) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │ *2  *2   .   .   .  *2 ││  .   1   .   .   .   1 │
  //  │                        ││                        │  *5 :
  //  │                        ││                        │
  //  │  .   .   .  *2   .   . ││ 13   .   .   .   .  *2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0,  0, 0, 1,  0, -2, 0, 0, 0,  -2, -2, 0, 0,
                                  0, -2, 0, 0, 13, 0, 0,  0, 0, -2, 0,  -5, 0, 0};
  Expectation<TypeParam> e(b, Dice(5, 6));
  e.add_expectation({{0, 0, VALUE}, {5, 1, VALUE}});
  e.add_expectation({{4, 0, VALUE}, {9, 1, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 2);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, double_stone_double_dice) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   1   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, -15, 0, 0};
  Expectation<TypeParam> e(b, Dice(3, 3));
  for (int32_t move_first = 0; move_first < 5; ++move_first) {
    int32_t move_second = 4 - move_first;
    e.add_expectation({{0, 0, VALUE},
                       {move_first * 3, 1, VALUE},
                       {1, 0, VALUE},
                       {1 + move_second * 3, 1, VALUE}});
  }
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 5);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, double_stone_double_dice_2) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   1   .   .   .   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  13 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 0, 0, 0, 1, 0, 0, 0, 0, 0,  0, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, -15, 0, 0};
  Expectation<TypeParam> e(b, Dice(4, 4));

  for (int32_t move_first = 0; move_first < 5; ++move_first) {
    auto move_second = 4 - move_first;
    e.add_expectation({{4, 0, VALUE},
                       {4 + move_second * 4, 1, DELTA},
                       {0, 0, VALUE},
                       {move_first * 4, 1, DELTA}});
  }
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 3);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, four_stones_double_dice) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   1   1   1   1 │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││  .   .   .   .   .  11 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  0, 0,   0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, -15, 0, 0};
  Expectation<TypeParam> e(b, Dice(4, 4));
  for (int32_t move_a = 0; move_a < 5; ++move_a) {
    for (int32_t move_b = 0; move_b < 5 - move_a; ++move_b) {
      for (int32_t move_c = 0; move_c < 5 - move_a - move_b; ++move_c) {
        int32_t move_d = 4 - move_a - move_b - move_c;

        e.add_expectation({{0, 0, VALUE},
                           {1, 0, VALUE},
                           {2, 0, VALUE},
                           {3, 0, VALUE},
                           {move_a * 4, 1, DELTA},
                           {1 + move_b * 4, 1, DELTA},
                           {2 + move_c * 4, 1, DELTA},
                           {3 + move_d * 4, 1, DELTA}});
      }
    }
  }
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 35);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, collect_with_waste) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │     :  12
  //  │  .   .   .   .   .   . ││  .   .   .   1   1   1 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, -15, 12, 0};
  Expectation<TypeParam> e(b, Dice(4, 5));
  e.add_expectation(
      {{WHITE_BAG, 14, VALUE}, {COLLECT_END - 3, 0, VALUE}, {COLLECT_END - 2, 0, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, collect_double) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │
  //  │  .   .   .   .   .   . ││ 10   1   1   1   1   1 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0,   0, 0,
                                  0, 0, 0, 0, 10, 1, 1, 1, 1, 1, 0, -15, 0, 0};
  Expectation<TypeParam> e(b, Dice(6, 6));
  e.add_expectation({{COLLECT_END - 6, 6, VALUE}, {WHITE_BAG, 4, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, move_in_collect_phase) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │     :  13
  //  │  .   .   .   .   .   . ││  1   .   .   .   .   1 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -15, 13, 0};
  Expectation<TypeParam> e(b, Dice(3, 2));
  e.add_expectation({{COLLECT_END - 6, 0, VALUE}, {COLLECT_END - 1, 2, VALUE}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, partial_collect) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *13 :
  //  │                        ││                        │     :  11
  //  │  .   .   .   .   .   . ││  .   1   .   1  *2   2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 1, 0, 1, -2, 2, 0, -13, 11, 0};
  Expectation<TypeParam> e(b, Dice(3, 3));
  e.add_expectation({{COLLECT_END - 3, 0, VALUE}, {WHITE_BAG, 1, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, move_and_collect) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *15 :
  //  │                        ││                        │     :  13
  //  │  .   .   .   .   .   . ││  1   .   .   1   .   . │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0,  0,
                                  0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, -15, 13, 0};
  Expectation<TypeParam> e(b, Dice(5, 3));
  e.add_expectation({{COLLECT_END - 6, 0, VALUE},
                     {COLLECT_END - 1, 1, VALUE},
                     {COLLECT_END - 3, 0, VALUE},
                     {WHITE_BAG, 1, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, move_all_no_collect) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  .   .   .   .   .   . ││  .   .   .   .   .   . │
  //  │                        ││                        │ *13 :
  //  │                        ││                        │     :  10
  //  │  .   .   .   .   .   . ││  .   .   .   5   .  *2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0,   0,  0,
                                  0, 0, 0, 0, 0, 0, 0, 5, 0, -2, 0, -13, 10, 0};
  Expectation<TypeParam> e(b, Dice(1, 1));
  e.add_expectation({{COLLECT_END - 3, -4, DELTA}, {COLLECT_END - 2, 4, DELTA}});
  auto r = e.get_test_result();
  EXPECT_EQ(r.size, 1);
  EXPECT_EQ(r.result, r.expectation);
}

TYPED_TEST(StateTest, game_end) {
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
    Expectation<TypeParam> e(b, dice);
    e.add_expectation({{COLLECT_END - 1, 0, VALUE},
                       {COLLECT_END - 3, 0, VALUE},
                       {COLLECT_END - 5, 0, VALUE},
                       {WHITE_BAG, 15, VALUE}});
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 1);
    EXPECT_EQ(r.result, r.expectation);
  }
}

TYPED_TEST(StateTest, count_moves_at_start) {
  //  ┌────────────────────────┬┬────────────────────────┐
  //  │  5   .   .   .  *3   . ││ *5   .   .   .   .   2 │
  //  │                        ││                        │
  //  │                        ││                        │
  //  │ *5   .   .   .   3   . ││  5   .   .   .   .  *2 │
  //  └────────────────────────┴┴────────────────────────┘
  auto b = std::array<int8_t, 28>{2, 0, 0, 0, 0, -5, 0, -3, 0, 0,  0, 5, -5, 0,
                                  0, 0, 3, 0, 5, 0,  0, 0,  0, -2, 0, 0, 0,  0};

  {
    Expectation<TypeParam> e(b, Dice(6, 6));
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 11);
  }

  {
    Expectation<TypeParam> e(b, Dice(5, 5));
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 4);
  }

  {
    Expectation<TypeParam> e(b, Dice(1, 5));
    auto r = e.get_test_result();
    EXPECT_EQ(r.size, 8);
  }
}

}  // namespace bg
