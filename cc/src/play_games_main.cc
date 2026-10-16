#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "CLI/CLI.hpp"
#include "assertion.h"
#include "configs.h"
#include "dice.h"
#include "game_manager.h"
#include "player.h"

namespace {

auto run(int argc, char** argv) -> int {
  using bg::FirstMove;
  using bg::GameManager;
  using bg::GameOptions;
  static constexpr int32_t DEFAULT_N_GAMES = 10000;
  static constexpr float DEFAULT_TEMPERATURE = 0.1;

  CLI::App app{"Play games for training"};

  GameOptions opt{.white_model = "random",
                  .black_model = "random",
                  .white_temperature = DEFAULT_TEMPERATURE,
                  .black_temperature = DEFAULT_TEMPERATURE,
                  .is_interactive = false,
                  .print_score = true,
                  .log_path = "",
                  .n_games = DEFAULT_N_GAMES,
                  .first_move = FirstMove::Alternate};

  app.add_option("--w_model", opt.white_model, "Path of the white model if white is of type NN)");
  app.add_option("--b_model", opt.black_model, "Path of the black model if black is of type NN)");
  app.add_option("--w_temperature", opt.white_temperature,
                 "Exploration of the white nn player; 0 always picks the best move");
  app.add_option("--b_temperature", opt.black_temperature,
                 "Exploration of the black nn player; 0 always picks the best move");
  app.add_option("--interactive", opt.is_interactive, "Print the board after every move");
  app.add_option("--print_score", opt.print_score, "Print the after every game");
  app.add_option("--log_path", opt.log_path, "Path to game logs");
  app.add_option("--n_games", opt.n_games, "Number of games to play");
  uint64_t seed = 0;
  auto* seed_option =
      app.add_option("--seed", seed, "Match seed (default: random); reproduces the whole match");

  auto* first_player_group = app.add_option_group("who_plays_first");
  first_player_group->require_option(0, 1);
  first_player_group->add_flag_callback(
      "--white_first", [&opt] -> void { opt.first_move = FirstMove::Always_White; },
      "It is always white to make the first move.");
  first_player_group->add_flag_callback(
      "--black_first", [&opt] -> void { opt.first_move = FirstMove::Always_Black; },
      "It is always black to make the first move.");
  first_player_group->add_flag_callback(
      "--take_turns", [&opt] -> void { opt.first_move = FirstMove::Alternate; },
      "white and black take turns to make the first move.");

  CLI11_PARSE(app, argc, argv);
  if (seed_option->count() > 0) {
    opt.seed = seed;
  }

  {
    GameManager game_manager = bg::game_manager_factory(opt);
    std::cout << "seed: " << game_manager.seed() << "\n";

    auto [white_score, black_score] = game_manager.run();

    std::cout << opt.n_games << " games played. White: " << white_score << " Black: " << black_score
              << "\n";
  }
  return 0;
}

}  // namespace

auto main(int argc, char** argv) -> int {
  try {
    return run(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
  } catch (...) {
    std::cerr << "error: unknown exception\n";
  }
  return 1;
}
