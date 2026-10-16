// Converts a games file (as written by play_games_main) into training arrays.
// See training_data.h for what an example is.

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "CLI/CLI.hpp"
#include "assertion.h"
#include "logger.h"
#include "training_data.h"

namespace {

auto run(int argc, char** argv) -> int {
  CLI::App app{"Convert game records into training arrays (.npy)"};
  std::string games_path;
  std::string out_dir;
  app.add_option("--games", games_path, "Games file written by play_games_main")->required();
  app.add_option("--out", out_dir, "Directory for the .npy files; created if missing")->required();
  CLI11_PARSE(app, argc, argv);

  std::ifstream in(games_path, std::ios::binary);
  ASSERT(in.is_open(), "cannot open the games file");
  std::filesystem::create_directories(out_dir);

  bg::TrainingExamples examples;
  bg::LogReader reader(in);
  bg::Game game;
  int32_t n_games = 0;
  while (reader.next(&game)) {
    bg::add_game(game, n_games++, &examples);
  }
  bg::write_examples(examples, out_dir);
  std::cout << n_games << " games, " << examples.size() << " examples\n";
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
