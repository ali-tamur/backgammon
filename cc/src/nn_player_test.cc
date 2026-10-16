// The network path: Model::Predict, the NN player and the game manager with
// NN players, using testdata/test_model.onnx, whose outputs are known by
// construction (see testdata/make_test_model.py).

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <random>
#include <vector>

#include "configs.h"
#include "dice.h"
#include "game_manager.h"
#include "logger.h"
#include "model.h"
#include "moves.h"
#include "player.h"
// After the project headers: assertion.h's FAIL must not redefine googletest's.
#include "gtest/gtest.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {
namespace {

const char* const TEST_MODEL = "cc/src/testdata/test_model.onnx";

// The formulas of make_test_model.py.
auto weight(size_t i, size_t j) -> double {
  return static_cast<double>(static_cast<int32_t>(((i * 7) + (j * 13)) % 17) - 8) / 8;
}
auto bias(size_t j) -> double { return (static_cast<double>(j) - 1.5) / 4; }

auto expected_probabilities(const Board& board) -> std::array<double, 4> {
  NN_INPUT x = board_to_nn_input(board);
  std::array<double, 4> logits{};
  for (size_t j = 0; j < 4; ++j) {
    logits[j] = bias(j);
    for (size_t i = 0; i < x.size(); ++i) {
      logits[j] += weight(i, j) * x[i];
    }
  }
  double sum = 0;
  for (double& z : logits) {
    z = std::exp(z);
    sum += z;
  }
  for (double& z : logits) {
    z /= sum;
  }
  return logits;
}

auto expected_value(const Board& board) -> double {
  auto p = expected_probabilities(board);
  return p[0] - p[2] + (2 * (p[1] - p[3]));
}

// Positions met in random games, from the mover's side.
auto some_positions(int32_t n_games) -> std::vector<Board> {
  std::mt19937 rng(11);
  std::uniform_int_distribution<int32_t> die(1, 6);
  std::vector<Board> positions;
  for (int32_t g = 0; g < n_games; ++g) {
    Board b = GAME_START;
    while (b[WHITE_BAG] < NUM_STONES) {
      positions.push_back(b);
      std::vector<State> next = generate(b, Dice(die(rng), die(rng)));
      std::uniform_int_distribution<size_t> pick(0, next.size() - 1);
      b = next[pick(rng)].b;
      b = flip(b);
    }
  }
  return positions;
}

TEST(ModelTest, InputAndOutputShapes) {
  Model model(TEST_MODEL);
  EXPECT_EQ(model.GetInputShape(), (std::vector<int64_t>{-1, 248}));
  EXPECT_EQ(model.GetInputName(), "board");
  EXPECT_EQ(model.GetOutputName(), "probabilities");
}

// Also checks the order of the outputs: white_win, white_gammon, black_win,
// black_gammon.
TEST(ModelTest, PredictMatchesTheNetworkComputedByHand) {
  Model model(TEST_MODEL);
  std::vector<Board> boards = some_positions(3);
  std::vector<NN_INPUT> inputs;
  for (const Board& b : boards) {
    inputs.push_back(board_to_nn_input(b));
  }
  std::vector<Model::Score> scores = model.Predict(inputs);
  ASSERT_EQ(scores.size(), boards.size());
  for (size_t k = 0; k < boards.size(); ++k) {
    auto p = expected_probabilities(boards[k]);
    EXPECT_NEAR(scores[k].white_win, p[0], 1e-5);
    EXPECT_NEAR(scores[k].white_gammon, p[1], 1e-5);
    EXPECT_NEAR(scores[k].black_win, p[2], 1e-5);
    EXPECT_NEAR(scores[k].black_gammon, p[3], 1e-5);
  }
}

TEST(NnPlayerTest, ExpectedValue) {
  Model::Score score{
      .white_win = 0.1F, .white_gammon = 0.2F, .black_win = 0.3F, .black_gammon = 0.4F};
  EXPECT_NEAR(Player<NN>::expected_value(score), 0.1 - 0.3 + (2 * (0.2 - 0.4)), 1e-6);
}

TEST(NnPlayerTest, GreedyPicksTheMoveWithTheHighestExpectedValue) {
  Player<NN> player(std::make_shared<std::mt19937>(1), TEST_MODEL, 0);
  std::mt19937 rng(5);
  std::uniform_int_distribution<int32_t> die(1, 6);
  int32_t checked = 0;
  for (const Board& b : some_positions(5)) {
    Dice dice(die(rng), die(rng));
    std::vector<State> candidates = generate(b, dice);
    if (candidates.size() < 2) {
      continue;
    }
    std::vector<double> values;
    for (const State& s : candidates) {
      values.push_back(expected_value(s.b));
    }
    std::vector<double> sorted = values;
    std::ranges::sort(sorted, std::greater<>());
    if (sorted[0] - sorted[1] < 1e-4) {
      continue;  // too close to call with float arithmetic
    }
    size_t best = std::ranges::max_element(values) - values.begin();
    EXPECT_EQ(player.move(b, dice), candidates[best].b);
    ++checked;
  }
  EXPECT_GT(checked, 100);
}

TEST(NnPlayerTest, TemperatureSamplesMovesByWeight) {
  constexpr float TEMPERATURE = 0.5;
  constexpr int32_t SAMPLES = 20000;
  Player<NN> player(std::make_shared<std::mt19937>(3), TEST_MODEL, TEMPERATURE);
  Board b = GAME_START;
  Dice dice(3, 1);
  std::vector<State> candidates = generate(b, dice);
  ASSERT_GT(candidates.size(), 5);

  std::map<Board, double> weights;
  double total = 0;
  for (const State& s : candidates) {
    double w = std::exp(expected_value(s.b) / TEMPERATURE);
    weights[s.b] = w;
    total += w;
  }
  std::map<Board, int32_t> counts;
  for (int32_t n = 0; n < SAMPLES; ++n) {
    ++counts[player.move(b, dice)];
  }
  for (const auto& [board, w] : weights) {
    double p = w / total;
    double sigma = std::sqrt(SAMPLES * p * (1 - p));
    EXPECT_NEAR(counts[board], SAMPLES * p, (5 * sigma) + 1);
  }
  EXPECT_EQ(counts.size(), weights.size());  // nothing outside the candidates
}

TEST(NnPlayerTest, SingleCandidateIsReturned) {
  Player<NN> player(std::make_shared<std::mt19937>(1), TEST_MODEL, 0.5);
  int32_t checked = 0;
  for (const Board& b : some_positions(2)) {
    for (int32_t d = 1; d <= 6; ++d) {
      std::vector<State> candidates = generate(b, Dice(d, d));
      if (candidates.size() == 1) {
        EXPECT_EQ(player.move(b, Dice(d, d)), candidates[0].b);
        ++checked;
      }
    }
  }
  EXPECT_GT(checked, 0);
}

TEST(NnPlayerTest, GameManagerPlaysNnAgainstRandom) {
  std::string log_path = std::filesystem::temp_directory_path() / "bg_nn_player_test.pb";
  std::filesystem::remove(log_path);  // left over by an interrupted run
  GameOptions options{};
  options.white_model = std::filesystem::absolute(TEST_MODEL);
  options.black_model = "random";
  options.white_temperature = 0;
  options.n_games = 4;
  options.log_path = log_path;
  options.is_unittest = true;
  {
    auto game_manager = game_manager_factory(options);
    game_manager.run();
  }
  std::ifstream file(log_path, std::ios::binary);
  LogReader reader(file);
  EXPECT_EQ(reader.read_all().size(), 4);
  std::filesystem::remove(log_path);
}

}  // namespace
}  // namespace bg
// NOLINTEND(readability-magic-numbers)
