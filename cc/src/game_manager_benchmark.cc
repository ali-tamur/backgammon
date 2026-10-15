#include <benchmark/benchmark.h>

#include <random>

#include "configs.h"
#include "dice.h"
#include "game_manager.h"
#include "player.h"

static void BM_GameManager(benchmark::State& state) {
  using bg::FirstMove;
  using bg::GameOptions;
  const auto num_games = static_cast<int32_t>(state.range(0));
  GameOptions opt{.white_model = "random",
                  .black_model = "random",
                  .temperature = 0.0,
                  .is_interactive = false,
                  .print_score = false,
                  .log_path = "",
                  .n_games = num_games,
                  .first_move = FirstMove::Random};

  auto game_manager = bg::game_manager_factory(opt);
  for (auto _ : state) {  // NOLINT(clang-analyzer-deadcode.DeadStores): benchmark idiom
    auto [white_score, black_score] = game_manager.run();
    benchmark::DoNotOptimize(white_score);
    benchmark::DoNotOptimize(black_score);
  }
}

BENCHMARK(BM_GameManager)->Arg(20000);

BENCHMARK_MAIN();  // missing type specifier
