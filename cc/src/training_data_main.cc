#include <exception>
#include <iostream>
#include <string>

#include "CLI/CLI.hpp"
#include "assertion.h"
#include "configs.h"
#include "training_data.h"

namespace {

auto run(int argc, char** argv) -> int {
  CLI::App app{"Convert game logs to numpy for training"};
  std::string white_model;
  std::string log_path;
  std::string output_path;

  app.add_option("--w_model", white_model, "Path of the white model if white is of type NN)");
  app.add_option("--log_path", log_path, "Path to game logs");
  app.add_option("--output_path", output_path, "Path to training data");

  CLI11_PARSE(app, argc, argv);

  if (!white_model.empty()) {
    white_model = PROJECT_ROOT + white_model;
  }

  ASSERT(!log_path.empty(), "log_path is required");
  log_path = PROJECT_ROOT + log_path;

  ASSERT(!output_path.empty(), "output_path is required");
  output_path = PROJECT_ROOT + output_path;

  bg::generate_training_data(white_model, log_path, output_path);
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
