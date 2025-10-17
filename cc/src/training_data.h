#pragma once

#include "configs.h"
#include "proto/game.pb.h"

namespace bg {

auto game_to_training_data(const Game& game, const std::string& model_path /*currently ignored*/)
    -> std::vector<TrainingData>;

void generate_training_data(const std::string& white_model /*currently ignored*/,
                            const std::string& log_path, const std::string& output_path);

}  // namespace bg