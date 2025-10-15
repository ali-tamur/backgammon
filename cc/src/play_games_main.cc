#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#include "CLI/CLI.hpp"
#include "configs.h"
#include "dice.h"
#include "game_manager.h"
#include "player.h"

auto main(int argc, char **argv) -> int {
  using namespace bg;
  static constexpr int32_t DEFAULT_N_GAMES = 10000;
  static constexpr float DEFAULT_TEMPERATURE = 0.1;

  CLI::App app{"Play games for training"};

  GameOptions opt{.white_name = "alice",
                  .black_name = "bob",
                  .white_model = "",
                  .black_model = "",
                  .temperature = DEFAULT_TEMPERATURE,
                  .is_interactive = false,
                  .print_score = true,
                  .log_path = "",
                  .n_games = DEFAULT_N_GAMES};

  app.add_option("--w_name", opt.white_name, "Name of the white player");
  app.add_option("--b_name", opt.black_name, "Name of the black player");
  app.add_option("--w_model", opt.white_model, "Path of the white model if white is of type NN)");
  app.add_option("--b_model", opt.black_model, "Path of the black model if black is of type NN)");
  app.add_option("--temperature", opt.temperature, "Exploration/explotation ratio for nn players");
  app.add_option("--is_interactive", opt.is_interactive, "Print the board after every move");
  app.add_option("--is_print_score", opt.print_score, "Print the after every game");
  app.add_option("--log_path", opt.log_path, "Path to game logs");
  app.add_option("--n_games", opt.n_games, "Number of games to play");

  if (opt.white_model != "") {
    opt.white_model = PROJECT_ROOT + opt.white_model;
  }
  if (opt.black_model != "") {
    opt.black_model = PROJECT_ROOT + opt.black_model;
  }
  if (opt.log_path != "") {
    opt.log_path = PROJECT_ROOT + opt.log_path;
  }

  CLI11_PARSE(app, argc, argv);
  GameManager game_manager = game_manager_factory(opt);
  auto [white_score, black_score] = game_manager.run();
  std::cout << opt.n_games << " games played. White: " << white_score << " Black: " << black_score
            << "\n";
  return 0;
}
