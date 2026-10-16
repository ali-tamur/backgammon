#include "training_data.h"

#include <filesystem>
#include <string>

#include "configs.h"
#include "npy.h"
// After the project headers: assertion.h's FAIL must not redefine googletest's.
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {
namespace {

auto board_of(const Position& position) -> Board {
  Board b{};
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    b[i] = static_cast<int8_t>(position.cell(static_cast<int32_t>(i)));
  }
  return b;
}

void set_cells(Position* position, const Board& b) {
  for (int8_t c : b) {
    position->add_cell(c);
  }
}

auto examples_board(const TrainingExamples& e, size_t i) -> Board {
  Board b{};
  std::copy_n(e.boards.begin() + static_cast<std::ptrdiff_t>(i * BOARD_SIZE), BOARD_SIZE,
              b.begin());
  return b;
}

TEST(TrainingDataTest, OutcomeForMover) {
  EXPECT_EQ(outcome_for_mover(1), MOVER_SINGLE);
  EXPECT_EQ(outcome_for_mover(2), MOVER_GAMMON);
  EXPECT_EQ(outcome_for_mover(3), MOVER_GAMMON);
  EXPECT_EQ(outcome_for_mover(-1), OPPONENT_SINGLE);
  EXPECT_EQ(outcome_for_mover(-2), OPPONENT_GAMMON);
  EXPECT_EQ(outcome_for_mover(-3), OPPONENT_GAMMON);
}

// Black moves first and white wins a gammon: black's position is flipped and
// labelled as the mover losing a gammon; white's is kept and labelled a win.
TEST(TrainingDataTest, BlackPositionsAreFlippedAndRelabelled) {
  Board after_black = GAME_START;
  after_black[23] = -1;  // black moved one stone from its 24-point
  after_black[17] = -1;
  Board after_white = after_black;
  after_white[0] = 1;  // white moved one stone from its 24-point
  after_white[4] = 1;

  Game game;
  game.set_white_first(false);
  game.set_final_score(2);
  set_cells(game.mutable_start(), GAME_START);
  Turn* black_turn = game.add_turns();
  black_turn->set_dice(64);
  set_cells(black_turn->mutable_after(), after_black);
  Turn* white_turn = game.add_turns();
  white_turn->set_dice(43);
  set_cells(white_turn->mutable_after(), after_white);
  ASSERT_EQ(board_of(game.turns(1).after()), after_white);

  TrainingExamples e;
  add_game(game, 7, &e);
  ASSERT_EQ(e.size(), 2);
  EXPECT_EQ(e.boards.size(), 2 * BOARD_SIZE);

  EXPECT_EQ(examples_board(e, 0), flip(after_black));
  EXPECT_EQ(e.labels[0], OPPONENT_GAMMON);
  EXPECT_EQ(e.points[0], -2);

  EXPECT_EQ(examples_board(e, 1), after_white);
  EXPECT_EQ(e.labels[1], MOVER_GAMMON);
  EXPECT_EQ(e.points[1], 2);

  EXPECT_EQ(e.turn[0], 0);
  EXPECT_EQ(e.turn[1], 1);
  EXPECT_EQ(e.n_turns[0], 2);
  EXPECT_EQ(e.game[0], 7);
  EXPECT_EQ(e.game[1], 7);
}

TEST(NpyTest, HeaderIsAlignedAndDescribesTheArray) {
  std::string two_d = npy_header<int8_t>({3, 28});
  EXPECT_EQ(two_d.size() % 64, 0);
  EXPECT_EQ(two_d.substr(0, 8), std::string("\x93NUMPY\x01\x00", 8));
  EXPECT_THAT(two_d,
              ::testing::HasSubstr("'descr': '|i1', 'fortran_order': False, 'shape': (3, 28), }"));
  EXPECT_EQ(two_d.back(), '\n');

  std::string one_d = npy_header<int16_t>({5});
  EXPECT_EQ(one_d.size() % 64, 0);
  EXPECT_THAT(one_d,
              ::testing::HasSubstr("'descr': '<i2', 'fortran_order': False, 'shape': (5,), }"));
}

TEST(NpyTest, FileIsHeaderPlusData) {
  auto path = std::filesystem::temp_directory_path() / "bg_npy_test.npy";
  std::vector<int16_t> data{1, -2, 3};
  write_npy<int16_t>(path, std::span(data), {3});
  EXPECT_EQ(std::filesystem::file_size(path),
            npy_header<int16_t>({3}).size() + 3 * sizeof(int16_t));
  std::filesystem::remove(path);
}

}  // namespace
}  // namespace bg
// NOLINTEND(readability-magic-numbers)
