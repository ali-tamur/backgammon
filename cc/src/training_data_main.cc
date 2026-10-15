
#include "CLI/CLI.hpp"
#include "assertion.h"
#include "configs.h"
#include "training_data.h"

auto main(int argc, char** argv) -> int {
  using namespace bg;

  CLI::App app{"Convert game logs to numpy for training"};
  std::string white_model;
  std::string log_path;
  std::string output_path;

  app.add_option("--w_model", white_model, "Path of the white model if white is of type NN)");
  app.add_option("--log_path", log_path, "Path to game logs");
  app.add_option("--output_path", output_path, "Path to training data");

  CLI11_PARSE(app, argc, argv);

  if (white_model != "") {
    white_model = PROJECT_ROOT + white_model;
  }

  ASSERT(log_path != "", "log_path is required");
  log_path = PROJECT_ROOT + log_path;

  ASSERT(output_path != "", "output_path is required");
  output_path = PROJECT_ROOT + output_path;

  generate_training_data(white_model, log_path, output_path);
  return 0;
};
