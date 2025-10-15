#include <benchmark/benchmark.h>

#include <random>

#include "configs.h"
#include "dice.h"
#include "game_manager.h"
#include "player.h"

static void BM_GameManager(benchmark::State &state) {
  using namespace bg;
  int32_t num_games = state.range(0);
  GameOptions opt;
  opt.white_model = "";
  opt.black_model = "";
  opt.white_name = "alice";
  opt.black_name = "bob";
  opt.n_games = num_games;
  opt.log_path = "";
  opt.temperature = 0.0;
  opt.is_interactive = false;
  opt.print_score = false;

  auto game_manager = game_manager_factory(opt);

  for (auto _ : state) {
    auto [white_score, black_score] = game_manager.run();
    benchmark::DoNotOptimize(white_score);
    benchmark::DoNotOptimize(black_score);
  }
}

BENCHMARK(BM_GameManager)->Arg(10000);

BENCHMARK_MAIN();  // missing type specifier
