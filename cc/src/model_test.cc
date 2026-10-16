#include "model.h"

#include <algorithm>
#include <cstddef>
#include <set>
#include <vector>

#include "configs.h"
// After the project headers: assertion.h's FAIL must not redefine googletest's.
#include "gtest/gtest.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {
namespace {

constexpr size_t POINT_SLOTS = 9;
constexpr size_t BAR_SLOTS = 5;
constexpr size_t BAG_SLOTS = 11;
constexpr size_t BARS = POINT_SLOTS * COLLECT_END;  // first slot of the mover's bar
constexpr size_t BAGS = BARS + (2 * BAR_SLOTS);     // first slot of the mover's bag
constexpr size_t INPUT_SIZE = BAGS + (2 * BAG_SLOTS);

auto hot_slots(const NN_INPUT& input) -> std::vector<size_t> {
  std::vector<size_t> hot;
  for (size_t i = 0; i < input.size(); ++i) {
    if (input[i] != 0.0F) {
      EXPECT_EQ(input[i], 1.0F) << "slot " << i;
      hot.push_back(i);
    }
  }
  return hot;
}

TEST(BoardToNnInputTest, StartPosition) {
  NN_INPUT input = board_to_nn_input(GAME_START);
  ASSERT_EQ(input.size(), INPUT_SIZE);
  ASSERT_EQ(INPUT_SIZE, 248);

  std::vector<size_t> expected;
  for (size_t point = 0; point < COLLECT_END; ++point) {
    // Stones -4..+4 occupy slots 0..8; stacks of 5 are clipped to 4.
    int32_t stones = std::clamp<int32_t>(GAME_START[point], -4, 4);
    expected.push_back((POINT_SLOTS * point) + static_cast<size_t>(stones + 4));
  }
  expected.push_back(BARS);              // mover's bar: 0
  expected.push_back(BARS + BAR_SLOTS);  // opponent's bar: 0
  expected.push_back(BAGS);              // mover's bag: 0
  expected.push_back(BAGS + BAG_SLOTS);  // opponent's bag: 0
  EXPECT_EQ(hot_slots(input), expected);
}

// The bug this guards against: negative counts used to share slots with
// positive ones, so an opponent's blot looked like three of the mover's stones.
TEST(BoardToNnInputTest, EveryPointCountHasItsOwnSlot) {
  for (size_t point = 0; point < COLLECT_END; ++point) {
    std::set<size_t> slots;
    for (int32_t stones = -4; stones <= 4; ++stones) {
      Board board{};
      board[point] = static_cast<int8_t>(stones);
      std::vector<size_t> hot = hot_slots(board_to_nn_input(board));
      ASSERT_EQ(hot.size(), COLLECT_END + 4);
      size_t slot = hot[point];
      EXPECT_EQ(slot, (POINT_SLOTS * point) + static_cast<size_t>(stones + 4))
          << "point " << point << ", stones " << stones;
      slots.insert(slot);
    }
    EXPECT_EQ(slots.size(), POINT_SLOTS) << "point " << point;
  }
}

// Bars and bags are stored with the opponent's counts negative; both sides
// are encoded as counts.
TEST(BoardToNnInputTest, BarsAndBags) {
  for (int32_t stones = 0; stones <= 4; ++stones) {
    Board board{};
    board[WHITE_BAR] = static_cast<int8_t>(stones);
    board[BLACK_BAR] = static_cast<int8_t>(-stones);
    std::vector<size_t> hot = hot_slots(board_to_nn_input(board));
    ASSERT_EQ(hot.size(), COLLECT_END + 4);
    EXPECT_EQ(hot[COLLECT_END], BARS + static_cast<size_t>(stones));
    EXPECT_EQ(hot[COLLECT_END + 1], BARS + BAR_SLOTS + static_cast<size_t>(stones));
  }
  for (int32_t stones = 0; stones <= 10; ++stones) {
    Board board{};
    board[WHITE_BAG] = static_cast<int8_t>(stones);
    board[BLACK_BAG] = static_cast<int8_t>(-stones);
    std::vector<size_t> hot = hot_slots(board_to_nn_input(board));
    ASSERT_EQ(hot.size(), COLLECT_END + 4);
    EXPECT_EQ(hot[COLLECT_END + 2], BAGS + static_cast<size_t>(stones));
    EXPECT_EQ(hot[COLLECT_END + 3], BAGS + BAG_SLOTS + static_cast<size_t>(stones));
  }
}

// Documents a known limitation of this encoding (to be redesigned): counts
// beyond the limits are indistinguishable.
TEST(BoardToNnInputTest, LargeCountsAreClipped) {
  Board four{};
  four[3] = -4;
  four[WHITE_BAR] = 4;
  four[WHITE_BAG] = 10;
  Board more = four;
  more[3] = -7;
  more[WHITE_BAR] = 6;
  more[WHITE_BAG] = 14;
  EXPECT_EQ(board_to_nn_input(four), board_to_nn_input(more));
}

TEST(BoardToNnInputTest, PositionAndBoardAgree) {
  Position position;
  for (int8_t cell : GAME_START) {
    position.add_cell(cell);
  }
  EXPECT_EQ(position_to_model_input(position), board_to_nn_input(GAME_START));
}

}  // namespace
}  // namespace bg
// NOLINTEND(readability-magic-numbers)
