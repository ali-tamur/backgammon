#pragma once

#include "configs.h"
#include "proto/game.pb.h"

namespace bg {
void game_to_training_data(const Game& game, const std::string& model_path /*currently ignored*/,
                           std::vector<TrainingData>& output);

}  // namespace bg