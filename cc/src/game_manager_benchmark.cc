#include <benchmark/benchmark.h>

#include "configs.h"
#include "dice.h"
#include "game_manager.h"
#include "moves.h"
#include "player.h"
#include "py_interface.h"
#include "state.h"
#include <random>

static void BM_GameManager(benchmark::State &state) {
  using namespace bg;
  int32_t num_games = state.range(0);

  // std::cout << "Test starts \n";
  std::mt19937 gen_{std::random_device{}()};
  auto alice = Player<RANDOM, WhiteConfig>("alice", gen_);
  auto bob = Player<RANDOM, BlackConfig>("bob", gen_);
  auto dice_throw = DiceThrow();
  GameOptions options{};
  options.white_name = alice.name;
  options.black_name = bob.name;
  options.is_interactive = false;
  options.will_log = false;
  auto white_move = [&alice](const Board &b, Dice d) -> auto {
    return alice.move(b, d);
  };
  auto black_move = [&bob](const Board &b, Dice d) -> auto {
    return bob.move(b, d);
  };
  auto game_manager = GameManager(white_move, black_move, dice_throw, options);

  for (auto _ : state) {
    auto [white_score, black_score] = game_manager.run(num_games);
    //    std::cout << "[" << white_score << "-" << black_score << "]\n";
    benchmark::DoNotOptimize(white_score);
    benchmark::DoNotOptimize(black_score);
  }
}

BENCHMARK(BM_GameManager)->Arg(10000);

BENCHMARK_MAIN();
