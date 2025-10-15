#include <benchmark/benchmark.h>

#include <random>

#include "configs.h"
#include "dice.h"
#include "game_manager.h"
#include "player.h"

static void BM_GameManager(benchmark::State &state) {
  using namespace bg;
  int32_t num_games = state.range(0);
  GameOptions opt{.white_name = "alice",
                  .black_name = "bob",
                  .white_model = "",
                  .black_model = "",
                  .temperature = 0.0,
                  .is_interactive = false,
                  .print_score = false,
                  .log_path = "",
                  .n_games = num_games};

  auto game_manager = game_manager_factory(opt);
  for (auto _ : state) {
    auto [white_score, black_score] = game_manager.run();
    benchmark::DoNotOptimize(white_score);
    benchmark::DoNotOptimize(black_score);
  }
}

BENCHMARK(BM_GameManager)->Arg(10000);

BENCHMARK_MAIN();  // missing type specifier
